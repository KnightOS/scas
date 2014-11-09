#include "errors.h"
#include "list.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

const char *get_error_string(error_t *error) {
	switch (error->code) {
		case ERROR_INVALID_INSTRUCTION:
			return "Invalid instruction";
		case ERROR_VALUE_TRUNCATED:
			return "Value truncated";
		case ERROR_INVALID_SYNTAX:
			return "Invalid syntax";
		case ERROR_INVALID_DIRECTIVE:
			return "Invalid directive";
		case ERROR_UNKNOWN_SYMBOL:
			return "Unknown symbol";
		case ERROR_BAD_FILE:
			return "Unable to open file for reading";
		case ERROR_TRAILING_END:
			return "Trailing end has no matching if";
		default:
			return NULL;
	}
}

const char *get_warning_string(warning_t *warning) {
	switch (warning->code) {
		default:
			return NULL;
	}
}

void add_error(list_t *errors, int code, size_t line_number, const char *line, int column, const char *file_name) {
	error_t *error = malloc(sizeof(error_t));
	error->code = code;
	error->line_number = line_number;
	error->file_name = malloc(strlen(file_name) + 1);
	strcpy(error->file_name, file_name);
	error->line = malloc(strlen(line) + 1);
	strcpy(error->line, line);
	error->column = column;
	list_add(errors, error);
}
