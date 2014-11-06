#include "directives.h"
#include "errors.h"
#include "expression.h"
#include <strings.h>
#include <ctype.h>
#include <stdint.h>

#define ERROR(ERROR_CODE, COLUMN) add_error(state.errors, ERROR_CODE, state.line_number, state.line, COLUMN, state.file_name);

struct directive {
	char *match;
	int(*function)(struct assembler_state state, char **argc, int argv);
};

int handle_db(struct assembler_state state, char **argc, int argv) {
	return 1;
}

/* Keep this alphabetized */
struct directive directives[] = {
	{ "db", handle_db }
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
			min = mid + 1;
		} else {
			max = mid - 1;
		}
	}
	return NULL;
}

int try_handle_directive(struct assembler_state state, char **line) {
	if (**line == '.' || **line == '#') {
		struct directive *d = find_directive(*line);
		if (d == NULL) {
			ERROR(ERROR_INVALID_DIRECTIVE, state.column);
			return 1;
		}
		return d->function(state, NULL, 0);
	}
	return 0;
}
