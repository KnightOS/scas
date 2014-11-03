#include "assembler.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include "list.h"
#include "objects.h"
#include "readline.h"
#include "stringop.h"
#include "instructions.h"
#include "match.h"
#include "errors.h"

#define ERROR(ERROR_CODE, COLUMN) add_error(state.errors, ERROR_CODE, state.line_number, _line, COLUMN, state.file_name);

struct assembler_state {
	object_t *object;
	area_t *current_area;
	instruction_set_t *instruction_set;
	int line_number;
	int column;
	const char *file_name;
	list_t *errors;
	list_t *warnings;
};

object_t *assemble(FILE *file, const char *file_name, instruction_set_t *set, list_t *errors, list_t *warnings) {
	struct assembler_state state = {
		create_object(),
		create_area("CODE"),
		set,
		0, 0,
		file_name,
		errors,
		warnings
	};

	list_add(state.object->areas, state.current_area);
	uint8_t *instruction_buffer = malloc(64 / 8);

	while (!feof(file)) {
		++state.line_number;
		char *line = read_line(file);
		char *_line = malloc(strlen(line) + 1);
		strcpy(_line, line);
		line = strip_comments(line);
		line = strip_whitespace(line, &state.column);
		if (strlen(line) == 0) {
			continue;
		}
		instruction_match_t *match = match_instruction(state.instruction_set, line);
		if (match == NULL) {
			ERROR(ERROR_INVALID_INSTRUCTION, state.column);
		} else {
			uint64_t instruction = match->instruction->value;
			int i;
			for (i = 0; i < match->operands->length; ++i) {
				operand_ref_t *ref = match->operands->items[i];
				instruction |= ref->op->value << (match->instruction->width - ref->shift - ref->op->width);
			}
			for (i = 0; i < match->immediate_values->length; ++i) {
				immediate_ref_t *ref = match->immediate_values->items[i];
				immediate_t *imm = find_instruction_immediate(match->instruction, ref->key);
				/* 
				 * TODO: Attempt to evaluate this right here and now, and add it if it fails
				 * Do NOT include symbols when evaluating here, that'll make it so we can't relocate this.
				 * Until then, we just assume it failed and add it to the list of late immediate values
				 */
				late_immediate_t *late_imm = malloc(sizeof(late_immediate_t));
				late_imm->address = state.current_area->data_length + (imm->shift / 8);
				late_imm->width = imm->width;
				late_imm->type = imm->type;
				late_imm->expression = ref->value_provided;
				list_add(state.current_area->late_immediates, late_imm);
			}
			int bytes_width = match->instruction->width / 8;
			for (i = 0; i < bytes_width; ++i) {
				instruction_buffer[bytes_width - i - 1] = instruction & 0xFF;
				instruction >>= 8;
			}
			/* Add completed instruction */
			append_to_area(state.current_area, instruction_buffer, bytes_width);
		}

		free(_line);
		free(line);
	}
	return state.object;
}
