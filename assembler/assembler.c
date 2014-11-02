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

object_t *assemble(FILE *file, instruction_set_t *set) {
	object_t *object = create_object();
	while (!feof(file)) {
		char *line = read_line(file);
		line = strip_whitespace(line);
		line = strip_comments(line);
		if (strlen(line) == 0) {
			continue;
		}
		instruction_match_t *match = match_instruction(set, line);
		if (match == NULL) {
			printf("No match found for %s\n", line);
		} else {
			printf(	"Match found for %s:\n"
					"\t%s (%08X)\n"
					, line, match->instruction->match, (unsigned int)match->instruction->value);
			int i;
			for (i = 0; i < match->immediate_values->length; ++i) {
				immediate_ref_t *ref = match->immediate_values->items[i];
				immediate_t *imm = find_instruction_immediate(match->instruction, ref->key);
				printf("\timm: key='%c', value='%s'"
						" (shift='%d', width='%d')\n"
						, ref->key, ref->value_provided, imm->shift, imm->width);
			}
			for (i = 0; i < match->operands->length; ++i) {
				operand_ref_t *op = match->operands->items[i];
				printf("\top: name='%s', value='%04X' (shift='%d', width='%d')\n"
						, op->op->match, (unsigned int)op->op->value, (int)op->shift, (int)op->op->width);
			}
		}

		free(line);
	}
	return object;
}
