#include "assembler.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include "list.h"
#include "log.h"
#include "objects.h"
#include "readline.h"
#include "stringop.h"
#include "instructions.h"
#include "match.h"
#include "errors.h"
#include "expression.h"
#include "directives.h"

#define ERROR(ERROR_CODE, COLUMN) add_error(state->errors, ERROR_CODE, \
		*(int*)stack_peek(state->line_number_stack), \
		state->line, COLUMN, stack_peek(state->file_name_stack));

#define MAP_SOURCE(LENGTH) add_source_map((source_map_t *)stack_peek(state->source_map_stack), \
		*(int*)stack_peek(state->line_number_stack), state->line, \
		state->current_area->data_length, LENGTH);

struct assembler_state state;

void transform_local_labels(tokenized_expression_t *expression, const char *last_global_label) {
	int i;
	for (i = 0; i < expression->tokens->length; ++i) {
		expression_token_t *token = expression->tokens->items[i];
		if (token->type == SYMBOL) {
			if (isdigit(token->symbol[0]) 
					&& token->symbol[strlen(token->symbol) - 1] == '$') {
				/* ASxxxx local label */
				const char *fmtstring = "_ASxxxx_%s@%s";
				char *temp = malloc((strlen(fmtstring) - 4) 
						+ strlen(token->symbol)
						+ strlen(last_global_label) 
						+ 1);
				sprintf(temp, fmtstring, token->symbol, last_global_label);
				temp[(strlen(fmtstring) - 4) 
						+ strlen(token->symbol)
						+ strlen(last_global_label)] = '\0';
				scas_log(L_DEBUG, "Transformed ASxxxx label from %s to %s", token->symbol, temp);
				free(token->symbol);
				token->symbol = temp;
			}
		}
	}
}

int try_empty_line(struct assembler_state *state, char **line) {
	return strlen(*line) == 0;
}

int try_add_label(struct assembler_state *state, char **line) {
	int i;
	for (i = 0; (*line)[i] && (*line)[i] != ':'; ++i) {
		int isvalid = isalnum((*line)[i]) || (*line)[i] == '_' || (*line)[i] == '$' ||
			(i == 0 && (*line)[i] == '.');
		if (!isvalid) {
			return 0;
		}
	}
	if ((*line)[i] != ':') {
		return 0;
	}
	/* Check for illegal label names */
	if (strncmp("$", *line, i) == 0) {
		// Don't allow "$" alone, it's a special case
		scas_log(L_ERROR, "Illegal ($)");
		return 0;
	}
	if (isdigit(**line) && (*line)[i - 1] != '$') {
		// Don't allow symbols to start with a number unless they're ASxxxx local labels
		return 0;
	}
	/* Add label */
	symbol_t *sym = malloc(sizeof(symbol_t));
	if (isdigit(**line) && (*line)[i - 1] == '$') {
		/* ASxxxx local label, give it a better name */
		char *temp = malloc(i + 1);
		strncpy(temp, *line, i);
		temp[i] = '\0';

		const char *fmtstring = "_ASxxxx_%s@%s";
		sym->name = malloc((strlen(fmtstring) - 4) 
				+ i 
				+ strlen(state->last_global_label) 
				+ 1);
		sprintf(sym->name, fmtstring, temp, state->last_global_label);
		sym->name[(strlen(fmtstring) - 4) 
				+ i 
				+ strlen(state->last_global_label)] = '\0';
		scas_log(L_DEBUG, "Adding ASxxxx local label %s", sym->name);
		free(temp);
	} else {
		sym->name = malloc(i + 1);
		strncpy(sym->name, *line, i);
		sym->name[i] = '\0';
		state->last_global_label = sym->name;
	}
	sym->type = SYMBOL_LABEL;
	sym->value = state->PC;
	sym->defined_address = state->current_area->data_length;
	sym->exported = 1; /* TODO: Support explicit export */
	list_add(state->current_area->symbols, sym);
	scas_log(L_DEBUG, "Added label '%s' (pre-linking value 0x%08X) from %s:%d", sym->name, sym->value,
			(char *)stack_peek(state->file_name_stack), *(int *)stack_peek(state->line_number_stack));
	/* Modify this line so that processing may continue */
	if ((*line)[i + 1] == ':') {
		/* TODO: Export this, it's an ASxxxx global label */
		++i;
	}
	memmove(*line, *line + i + 1, strlen(*line + i));
	int _;
	*line = strip_whitespace(*line, &_);
	return strlen(*line) == 0;
}

int try_match_instruction(struct assembler_state *state, char **_line) {
	char *line = *_line;
	instruction_match_t *match = match_instruction(state->instruction_set, line);
	if (match == NULL) {
		ERROR(ERROR_INVALID_INSTRUCTION, state->column);
		return 0;
	} else {
		scas_log(L_DEBUG, "Matched string '%s' to instruction '%s'", line, match->instruction->match);
		indent_log();
		uint64_t instruction = match->instruction->value;
		int i;
		for (i = 0; i < match->operands->length; ++i) {
			operand_ref_t *ref = match->operands->items[i];
			scas_log(L_DEBUG, "Using operand '%s'", ref->op->match);
			instruction |= ref->op->value << (match->instruction->width - ref->shift - ref->op->width);
		}
		for (i = 0; i < match->immediate_values->length; ++i) {
			immediate_ref_t *ref = match->immediate_values->items[i];
			immediate_t *imm = find_instruction_immediate(match->instruction, ref->key);

			int error;
			uint64_t result;
			tokenized_expression_t *expression = parse_expression(ref->value_provided);
			if (expression == NULL) {
				error = EXPRESSION_BAD_SYNTAX;
			} else {
				transform_local_labels(expression, state->last_global_label);
				result = evaluate_expression(expression, state->equates, &error);
			}
			if (error == EXPRESSION_BAD_SYMBOL) {
				/* TODO: Throw error if using explicit import */
				scas_log(L_DEBUG, "Postponing evaluation of '%s' to linker", ref->value_provided);
				late_immediate_t *late_imm = malloc(sizeof(late_immediate_t));
				late_imm->instruction_address = state->current_area->data_length;
				late_imm->base_address = state->current_area->data_length + (match->instruction->width / 8);
				late_imm->address = state->current_area->data_length + (imm->shift / 8);
				late_imm->width = imm->width;
				late_imm->type = imm->type;
				late_imm->expression = expression;
				list_add(state->current_area->late_immediates, late_imm);
			} else if (error == EXPRESSION_BAD_SYNTAX) {
				ERROR(ERROR_INVALID_SYNTAX, state->column);
			} else {
				if (imm->type == IMM_TYPE_RELATIVE) {
					result = result - (state->PC + (match->instruction->width / 8));
				} else if (imm->type == IMM_TYPE_RESTART) {
					if ((result & ~0x07) != result || result > 0x38) {
						/* We get an ERROR_VALUE_TRUNCATED if we just let it proceed */
					} else {
						result >>= 3;
					}
				}
				scas_log(L_DEBUG, "Resolved '%s' early with result 0x%08X", ref->value_provided, result);
				uint64_t mask = 1;
				int shift = imm->width;
				while (--shift) {
					mask <<= 1;
					mask |= 1;
				}
				if ((result & mask) != result && ~result >> imm->width) {
					ERROR(ERROR_VALUE_TRUNCATED, state->column);
				} else {
					result = result & mask;
					int bits = imm->width;
					while (bits > 0) {
						bits -= 8;
						if (bits < 0) {
							bits = 0;
						}
						instruction |= (result & 0xFF) << (match->instruction->width - imm->shift - imm->width) << bits;
						result >>= 8;
					}
				}
			}
		}
		int bytes_width = match->instruction->width / 8;
		for (i = 0; i < bytes_width; ++i) {
			state->instruction_buffer[bytes_width - i - 1] = instruction & 0xFF;
			instruction >>= 8;
		}
		/* Add completed instruction */
		MAP_SOURCE(bytes_width);
		append_to_area(state->current_area, state->instruction_buffer, bytes_width);
		state->PC += bytes_width;
		deindent_log();
	}
	return 1;
}

char *split_line(struct assembler_state *state, char *line) {
	int i, j, _;
	for (i = 0, j = 0; line[i]; ++i) {
		if (line[i] == '\\') {
			char *part = malloc(i - j + 1);
			strncpy(part, line + j, i - j);
			part[i - j + 1] = '\0';
			part = strip_whitespace(part, &_);
			stack_push(state->extra_lines, part);
			j = i + 1;
		}
	}
	char *part = malloc(i - j + 1);
	strncpy(part, line + j, i - j);
	part[i - j + 1] = '\0';
	part = strip_whitespace(part, &_);
	free(line);
	return part;
}

object_t *assemble(FILE *file, const char *file_name, assembler_settings_t *settings) {
	struct assembler_state state = {
		.object = create_object(),
		.current_area = create_area("CODE"),
		.source_map_stack = create_stack(),

		.file_stack = create_stack(),
		.file_name_stack = create_stack(),
		.line_number_stack = create_stack(),
		.errors = settings->errors,
		.warnings = settings->warnings,

		.extra_lines = create_stack(),
		.line = "",
		.column = 0,

		.instruction_set = settings->set,
		.instruction_buffer = malloc(64 / 8),
		.if_stack = create_stack(),
		.equates = create_list(),
		.nolist = 0,
		.PC = 0,
		.last_global_label = "@start",
		.settings = settings,
	};
	int *ln = malloc(sizeof(int)); *ln = 0;
	char *name = malloc(strlen(file_name) + 1);
	strcpy(name, file_name);
	stack_push(state.file_name_stack, name);
	stack_push(state.line_number_stack, ln);
	stack_push(state.file_stack, file);
	stack_push(state.source_map_stack, create_source_map(state.current_area, file_name));
	list_add(state.object->areas, state.current_area);

	int(*const line_ops[])(struct assembler_state *, char **) = {
		try_empty_line,
		try_add_label,
		try_handle_directive,
		try_match_instruction,
	};
	int(*const nolist_line_ops[])(struct assembler_state *, char **) = {
		try_empty_line,
		try_handle_directive,
	};

	char *line;
	FILE *cur;
	while ((cur = stack_peek(state.file_stack))) {
		if (!feof(cur) || state.extra_lines->length) {
			if (state.extra_lines->length == 0) {
				++*(int*)stack_peek(state.line_number_stack);
				line = read_line(cur);
			} else {
				line = stack_pop(state.extra_lines);
			}
			state.line = malloc(strlen(line) + 1);
			strcpy(state.line, line);
			line = strip_comments(line);
			line = strip_whitespace(line, &state.column);
			if (code_strchr(line, '\\')) {
				line = split_line(&state, line);
			}
			int i;
			for (i = 0; i < sizeof(line_ops) / sizeof(void*); ++i) {
				if (state.nolist || (state.if_stack->length && !*(int*)stack_peek(state.if_stack))) {
					if (nolist_line_ops[i](&state, &line)) {
						break;
					}
				} else {
					if (line_ops[i](&state, &line)) {
						break;
					}
				}
			}
			free(state.line);
			free(line);
		} else {
			free(stack_pop(state.file_name_stack));
			free(stack_pop(state.line_number_stack));
			if (cur != file) {
				fclose(cur);
				stack_pop(state.file_stack);
				stack_pop(state.source_map_stack);
				scas_log(L_INFO, "Returning to file '%s'", (char *)stack_peek(state.file_name_stack));
			} else {
				break;
			}
		}
	}

	while (state.if_stack->length != 0) {
		free(stack_pop(state.if_stack));
	}
	stack_free(state.if_stack);
	stack_free(state.file_name_stack);
	stack_free(state.line_number_stack);
	stack_free(state.extra_lines);
	stack_free(state.source_map_stack);
	free(state.instruction_buffer);

	return state.object;
}
