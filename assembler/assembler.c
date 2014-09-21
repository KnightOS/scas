#include "assembler.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include "list.h"
#include "objects.h"
#include "readline.h"
#include "stringop.h"

object_t *assemble(FILE *file) {
	object_t *object = create_object();
	while (!feof(file)) {
		char *line = read_line(file);
		line = strip_whitespace(line);
		line = strip_comments(line);

		free(line);
	}
	return object;
}
