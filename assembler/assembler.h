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
	int line_number;
	int column;
	const char *file_name;
	list_t *errors;
	list_t *warnings;
	char *line;
	uint8_t *instruction_buffer;
	/* Lines pushed to this will be used before drawing more from the file */
	stack_t *extra_lines;
};

object_t *assemble(FILE *file, const char *file_name, instruction_set_t *set, list_t *errors, list_t *warnings);

#endif
