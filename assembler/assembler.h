#ifndef ASSEMBLER_H
#define ASSEMBLER_H
#include <stdio.h>
#include "list.h"
#include "objects.h"
#include "instructions.h"

typedef struct {
	instruction_set_t *set;
	list_t *include_path;
	list_t *errors;
	list_t *warnings;
} assembler_settings_t;

typedef struct {
	char *name;
	list_t/*char **/ *macro_lines;
	list_t/*char **/ *parameters;
} macro_t;

struct assembler_state {
	object_t *object;
	area_t *current_area;
	stack_t *source_map_stack;
	stack_t *file_stack;
	stack_t *file_name_stack;
	stack_t *line_number_stack;
	list_t *errors;
	list_t *warnings;
	list_t *extra_lines;
	char *line;
	int column;
	instruction_set_t *instruction_set;
	uint8_t *instruction_buffer;
	stack_t *if_stack;
	list_t *equates;
	list_t *macros;
	macro_t *current_macro;
	int nolist;
	uint64_t PC;
	char *last_global_label;
	assembler_settings_t *settings;
};

object_t *assemble(FILE *file, const char *file_name, assembler_settings_t *settings);

#endif
