#include "instructions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"
#include "readline.h"
#include "stringop.h"

operand_group_t *find_operand_group(instruction_set_t *set, const char *name) {
	int i;
	for (i = 0; i < set->operand_groups->length; ++i) {
		operand_group_t *g = set->operand_groups->items[i];
		if (strcmp(g->name, name) == 0) {
			return g;
		}
	}
	return NULL;
}

operand_group_t *create_operand_group(const char *name) {
	operand_group_t *g = malloc(sizeof(operand_group_t));
	g->name = malloc(strlen(name) + 1);
	strcpy(g->name, name);
	g->operands = create_list();
	return g;
}

operand_t *create_operand(const char *match, uint64_t val, size_t len) {
	operand_t *op = malloc(sizeof(operand_t));
	op->match = malloc(strlen(match) + 1);
	strcpy(op->match, match);
	op->value = val;
	op->width = len;
	return op;
}

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
			list_t *parts = split_string(line, " \t");
			if (parts->length != 4) {
				fprintf(stderr, "Warning: Skipping invalid definition from instruction set: %s\n", line);
				list_free(parts);
				continue;
			}
			operand_group_t *g = find_operand_group(result, (char *)parts->items[1]);
			if (g == NULL) {
				g = create_operand_group((char *)parts->items[1]);
				list_add(result->operand_groups, g);
			}
			char *end;
			uint64_t val = (uint64_t)strtol((char *)parts->items[3], &end, 2);
			if (*end != '\0') {
				fprintf(stderr, "Warning: Skipping invalid definition from instruction set: %s\n", line);
				list_free(parts);
				continue;
			}
			operand_t *op = create_operand((char *)parts->items[2], val, strlen((char *)parts->items[3]));
			list_add(g->operands, op);
			list_free(parts);
		}
		if (strstr(line, "INS ") == line) {
			list_t *parts = split_string(line, " \t");
			if (parts->length != 2) {
				fprintf(stderr, "Warning: Skipping invalid definition from instruction set: %s\n", line);
				list_free(parts);
				continue;
			}
			/* 
			 * TODO: We need to split up and parse that instruction, it'll be a bit complicated
			 * We need to parse the match to figure out the bit width of the various substitutions
			 * We don't want to keep the value strings around, it'll make assembly faster that way
			 * So, we need to first iterate over the match and identify the width of each match string
			 * Then do a substitution in the value string for `000...` with the appropriate width
			 * And then it's easy, just strtol the value and store it with the match in the instruction set
			 */
			list_free(parts);
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
