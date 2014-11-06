#include "directives.h"
#include "errors.h"
#include "expression.h"
#include <stdint.h>

#define ERROR(ERROR_CODE, COLUMN) add_error(state.errors, ERROR_CODE, state.line_number, state.line, COLUMN, state.file_name);

struct directive {
	char *match;
	int(*function)(struct assembler_state state, char *line);
};

int handle_db(struct assembler_state state, char *line) {
	return 0;
}

struct directive *find_directive(char *line) {
	++line;
	/* TODO: Binary search */
	return NULL;
}

/* Keep this alphabetized */
struct directive directives[] = {
	{ "db", handle_db }
};

int try_handle_directive(struct assembler_state state, char **line) {
	if (**line == '.' || **line == '#') {
		struct directive *d = find_directive(*line);
		if (d == NULL) {
			ERROR(ERROR_INVALID_DIRECTIVE, state.column);
			return 1;
		}
		return d->function(state, *line);
	}
	return 0;
}
