#include "instructions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"
#include "readline.h"
#include "stringop.h"

instruction_set_t *load_instruction_set(FILE *file) {
	instruction_set_t *result = malloc(sizeof(instruction_set_t));
	result->instructions = create_list();
	result->arch = NULL;
	while (!feof(file)) {
		char *line = read_line(file);
		line = strip_whitespace(line);
		if (line[0] == '\0' || line[0] == '#') {
			free(line);
			continue;
		}
		if (strstr(line, "ARCH ") == line) {
			result->arch = malloc(strlen(line) - 4);
			strcpy(result->arch, line + 5);
		}
		free(line);
	}
	return result;
}

void instruction_set_free(instruction_set_t *set) {
	int i;
	for (i = 0; i < set->instructions->length; ++i) {
		instruction_t *inst = set->instructions->items[i];
		free(inst->match);
		free(inst);
	}
	list_free(set->instructions);
	if (set->arch != NULL) {
		free(set->arch);
	}
	free(set);
}
