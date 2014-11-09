#include "assembler.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include "list.h"
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

struct assembler_state state;

int try_empty_line(struct assembler_state *state, char **line) {
	return strlen(*line) == 0;
}

int try_add_label(struct assembler_state *state, char **line) {
	int i;
	for (i = 0; (*line)[i] && (*line)[i] != ':'; ++i) {
		int isvalid = isalnum((*line)[i]) || (*line)[i] == '_' ||
			(i == 0 && (*line)[i] == '.');
		if (!isvalid) {
			return 0;
		}
	}
	if ((*line)[i] != ':') {
		return 0;
	}
	/* Add label */
	symbol_t *sym = malloc(sizeof(symbol_t));
	sym->name = malloc(i + 1);
	strncpy(sym->name, *line, i);
	sym->name[i] = '\0';
	sym->type = SYMBOL_LABEL;
	sym->value = state->PC;
	sym->defined_address = state->current_area->data_length;
	sym->exported = 1; /* TODO: Support explicit export */
	list_add(state->current_area->symbols, sym);
	/* Modify this line so that processing may continue */
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
		uint64_t instruction = match->instruction->value;
		int i;
		for (i = 0; i < match->operands->length; ++i) {
			operand_ref_t *ref = match->operands->items[i];
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
				result = evaluate_expression(expression, state->equates, &error);
			}
			if (error == EXPRESSION_BAD_SYMBOL) {
				/* TODO: Throw error if using explicit import */
				late_immediate_t *late_imm = malloc(sizeof(late_immediate_t));
				late_imm->address = state->current_area->data_length + (imm->shift / 8);
				late_imm->width = imm->width;
				late_imm->type = imm->type;
				late_imm->expression = expression;
				list_add(state->current_area->late_immediates, late_imm);
			} else if (error == EXPRESSION_BAD_SYNTAX) {
				ERROR(ERROR_INVALID_SYNTAX, state->column);
			} else {
				if (imm->type == IMM_TYPE_RELATIVE) {
					result += state->PC;
				}
				/* TODO: Handle IMM_TYPE_RESTART */
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
					while (bits) {
						bits -= 8;
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
		append_to_area(state->current_area, state->instruction_buffer, bytes_width);
		state->PC += bytes_width;
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

object_t *assemble(FILE *file, const char *file_name, instruction_set_t *set, list_t *errors, list_t *warnings) {
	struct assembler_state state = {
		.object = create_object(),
		.current_area = create_area("CODE"),

		.file_stack = create_stack(),
		.file_name_stack = create_stack(),
		.line_number_stack = create_stack(),
		.errors = errors,
		.warnings = warnings,

		.extra_lines = create_stack(),
		.line = "",
		.column = 0,

		.instruction_set = set,
		.instruction_buffer = malloc(64 / 8),
		.if_stack = create_stack(),
		.equates = create_list(),
		.nolist = 0,
		.PC = 0,
	};
	int *ln = malloc(sizeof(int)); *ln = 0;
	int *_if = malloc(sizeof(int)); *_if = 1;
	char *name = malloc(strlen(file_name) + 1);
	strcpy(name, file_name);
	stack_push(state.file_name_stack, name);
	stack_push(state.line_number_stack, ln);
	stack_push(state.file_stack, file);
	stack_push(state.if_stack, _if);

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
				if (state.nolist || !*(int*)stack_peek(state.if_stack)) {
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
	free(state.instruction_buffer);

	return state.object;
}
