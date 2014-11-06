#include "directives.h"
#include "errors.h"
#include "expression.h"
#include "stringop.h"
#include "objects.h"
#include "list.h"
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>

#define ERROR(ERROR_CODE, COLUMN) add_error(state->errors, ERROR_CODE, state->line_number, state->line, COLUMN, state->file_name);

struct directive {
	char *match;
	int(*function)(struct assembler_state *state, char **argv, int argc);
};

int handle_area(struct assembler_state *state, char **argv, int argc) {
	if (argc != 1) {
		ERROR(ERROR_INVALID_DIRECTIVE, state->column);
		return 1;
	}
	area_t *area = NULL;
	int i;
	for (i = 0; i < state->object->areas->length; ++i) {
		area_t *a = state->object->areas->items[i];
		if (strcasecmp(a->name, argv[0]) == 0) {
			area = a;
			break;
		}
	}
	if (!area) {
		area = create_area(argv[0]);
		list_add(state->object->areas, area);
	}
	state->current_area = area;
	return 1;
}

int handle_db(struct assembler_state *state, char **argv, int argc) {
	if (argc == 0) {
		ERROR(ERROR_INVALID_DIRECTIVE, state->column);
		return 1;
	}
	int i;
	for (i = 0; i < argc; ++i) {
		int len = strlen(argv[i]);
		if (argv[i][0] == '"' && argv[i][len - 1] == '"') {
			/* TODO: Do we need to do anything fancy wrt encoding? */
			argv[i][len - 1] = '\0';
			len -= 2;
			len = unescape_string(argv[i] + 1);
			append_to_area(state->current_area, (unsigned char*)(argv[i] + 1), len);
		} else {
			tokenized_expression_t *expression = parse_expression(argv[i]);
			int error;
			uint64_t result = evaluate_expression(expression, NULL /* TODO: Symbols */, &error);
			if (error == EXPRESSION_BAD_SYMBOL) {
				/* TODO: Throw error if using explicit import */
				late_immediate_t *late_imm = malloc(sizeof(late_immediate_t));
				late_imm->address = state->current_area->data_length;
				late_imm->width = 8;
				late_imm->type = IMM_TYPE_ABSOLUTE;
				late_imm->expression = expression;
				list_add(state->current_area->late_immediates, late_imm);
				*state->instruction_buffer = 0;
			} else if (error == EXPRESSION_BAD_SYNTAX) {
				ERROR(ERROR_INVALID_SYNTAX, state->column);
			} else {
				if ((result & 0xFF) != result) {
					ERROR(ERROR_VALUE_TRUNCATED, state->column);
				} else {
					*state->instruction_buffer = (uint8_t)result;
				}
			}
			append_to_area(state->current_area, state->instruction_buffer, 1);
		}
	}
	return 1;
}

/* Keep this alphabetized */
struct directive directives[] = {
	{ "area", handle_area },
	{ "db", handle_db },
};

struct directive *find_directive(char *line) {
	++line;
	int whitespace = 0;
	while (line[whitespace] && !isspace(line[whitespace++])); --whitespace;
	int length = sizeof(directives) / sizeof(struct directive);
	int min = 0, max = length - 1;
	while (max >= min) {
		int mid = (max - min) / 2;
		int cmp = strncasecmp(directives[mid].match, line, whitespace);
		if (cmp == 0) {
			return &directives[mid];
		} else if (cmp < 0) {
			min = mid - 1;
		} else {
			max = mid + 1;
		}
	}
	return NULL;
}

char **split_directive(char *line, int *argc) {
	*argc = 0;
	while (isspace(*line) && *line) ++line;
	/*
	 * Directives can be delimited with whitespace OR with commas.
	 * If you use commas at all, commas will be used exclusively.
	 */
	int capacity = 10;
	char **parts = malloc(sizeof(char *) * capacity);
	char *delimiters;
	if (code_strchr(line, ',') == NULL) {
		delimiters = "\t ";
	} else {
		delimiters = ",";
	}
	int in_string = 0, in_character = 0;
	int i, j, _;
	for (i = 0, j = 0; line[i]; ++i) {
		if (line[i] == '\\') {
			++i;
		} else if (line[i] == '"' && !in_character) {
			in_string = !in_string;
		} else if (line[i] == '\'' && !in_string) {
			in_character = !in_character;
		} else if (!in_character && !in_string) {
			if (strchr(delimiters, line[i]) != NULL) {
				char *item = malloc(i - j + 1);
				strncpy(item, line + j, i - j);
				item[i - j] = '\0';
				item = strip_whitespace(item, &_);
				if (*argc == capacity) {
					capacity *= 2;
					parts = realloc(parts, sizeof(char *) * capacity);
				}
				parts[*argc] = item;
				j = i + 1;
				++*argc;
			}
		}
	}
	char *item = malloc(i - j);
	strncpy(item, line + j, i - j);
	item[i - j] = '\0';
	item = strip_whitespace(item, &_);
	if (*argc == capacity) {
		capacity++;
		parts = realloc(parts, sizeof(char *) * capacity);
	}
	parts[*argc] = item;
	++*argc;
	return parts;
}

int try_handle_directive(struct assembler_state *state, char **line) {
	if (**line == '.' || **line == '#') {
		struct directive *d = find_directive(*line);
		if (d == NULL) {
			ERROR(ERROR_INVALID_DIRECTIVE, state->column);
			return 1;
		}
		int argc;
		char **argv = split_directive(*line + strlen(d->match) + 1, &argc);
		int ret = d->function(state, argv, argc);
		while (argc--) {
			free(argv[argc]);
		}
		free(argv);
		return ret;
	}
	return 0;
}
