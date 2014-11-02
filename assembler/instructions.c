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
	result->operand_groups = create_list();
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
		if (strstr(line, "OPERAND ") == line) {
			/* TODO */
		}
		if (strstr(line, "INS ") == line) {
			/* TODO */
		}
		free(line);
	}
	return result;
}

void instruction_set_free(instruction_set_t *set) {
	int i, n;
	for (i = 0; i < set->instructions->length; ++i) {
		instruction_t *inst = set->instructions->items[i];
		free(inst->match);
		free(inst);
	}
	list_free(set->instructions);

	for (i = 0; i < set->operand_groups->length; ++i) {
		operand_group_t *group = set->operand_groups->items[i];
		for (n = 0; n < group->operands->length; ++n) {
			operand_t *op = group->operands->items[n];
			free(op->match);
			free(op);
		}
		free(group->name);
		free(group);
	}
	list_free(set->operand_groups);

	if (set->arch != NULL) {
		free(set->arch);
	}
	free(set);
}

instruction_t *match_instruction(const char *text) {
	/* TODO */
	return NULL;
}
