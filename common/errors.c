#include "errors.h"
#include "list.h"
#include "log.h"
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
			return "No matching if directive";
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
	scas_log(L_ERROR, "Added error '%s' at %s:%d:%d", get_error_string(error), file_name, line_number, column);
}

/* Locates the address in the source maps provided to get the file name and line number */
void add_error_from_map(list_t *errors, int code, list_t *maps, uint64_t address) {
	// TODO
	error_t *error = malloc(sizeof(error_t));
	error->code = code;
	error->line_number = 0;
	error->file_name = NULL;
	error->line = NULL;
	error->column = 0;
	list_add(errors, error);
}
