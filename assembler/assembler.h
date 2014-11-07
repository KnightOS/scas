#ifndef ASSEMBLER_H
#define ASSEMBLER_H
#include <stdio.h>
#include "list.h"
#include "objects.h"
#include "instructions.h"

struct assembler_state {
	object_t *object;
	area_t *current_area;
	list_t *equates;
	instruction_set_t *instruction_set;
	stack_t *line_number_stack;
	int column;
	stack_t *file_name_stack;
	stack_t *file_stack;
	list_t *errors;
	list_t *warnings;
	char *line;
	uint8_t *instruction_buffer;
	stack_t *extra_lines;
	int nolist;
};

object_t *assemble(FILE *file, const char *file_name, instruction_set_t *set, list_t *errors, list_t *warnings);

#endif
