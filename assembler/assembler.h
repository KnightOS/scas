#ifndef ASSEMBLER_H
#define ASSEMBLER_H
#include <stdio.h>
#include "list.h"
#include "objects.h"
#include "instructions.h"

struct assembler_state {
	object_t *object;
	area_t *current_area;
	stack_t *source_map_stack;
	stack_t *file_stack;
	stack_t *file_name_stack;
	stack_t *line_number_stack;
	list_t *errors;
	list_t *warnings;
	stack_t *extra_lines;
	char *line;
	int column;
	instruction_set_t *instruction_set;
	uint8_t *instruction_buffer;
	stack_t *if_stack;
	list_t *equates;
	int nolist;
	uint64_t PC;
};

object_t *assemble(FILE *file, const char *file_name, instruction_set_t *set, list_t *errors, list_t *warnings);

#endif
