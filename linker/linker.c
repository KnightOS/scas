#include "linker.h"
#include "objects.h"
#include "errors.h"
#include "list.h"
#include "expression.h"
#include "instructions.h"
#include <stdint.h>
#include <stdlib.h>
#include <strings.h>

/*
 * Notes on how this could be improved:
 * A sorted list_t would make a lot of these lookups faster
 * A hashtable could also be used to handle dupes and have fast lookup
 */

typedef struct {
	char *name;
	uint64_t address;
	uint64_t length;
	list_t *areas;
} area_state_t;

area_state_t *find_area_state(list_t *areas, char *name) {
	int i;
	for (i = 0; i < areas->length; ++i) {
		area_state_t *a = areas->items[i];
		if (strcasecmp(a->name, name) == 0) {
			return a;
		}
	}
	return NULL;
}

area_state_t *create_area_state(char *name) {
	area_state_t *as = malloc(sizeof(area_state_t));
	as->name = name;
	as->length = 0;
	as->address = 0;
	as->areas = create_list();
	return as;
}

symbol_t *find_symbol(list_t *symbols, char *name) {
	int i;
	for (i = 0; i < symbols->length; ++i) {
		symbol_t *sym = symbols->items[i];
		if (strcasecmp(sym->name, name) == 0) {
			return sym;
		}
	}
	return NULL;
}

/* TODO: Error handling in this code sucks */

void gather_and_relocate_symbols(list_t *symbols, area_t *area, list_t *errors) {
	int i;
	for (i = 0; i < area->symbols->length; ++i) {
		symbol_t *sym = area->symbols->items[i];
		symbol_t *existing = find_symbol(symbols, sym->name);
		if (existing) {
			add_error(errors, ERROR_DUPLICATE_SYMBOL, 0, sym->name, 0, NULL);
			continue;
		}
		if (sym->type == SYMBOL_LABEL) {
			sym->value += area->final_address;
			list_add(symbols, sym);
		}
	}
}

void resolve_immediate_values(list_t *symbols, area_t *area, list_t *errors) {
	int i;
	for (i = 0; i < area->late_immediates->length; ++i) {
		late_immediate_t *imm = area->late_immediates->items[i];
		fprintf(stderr, "Resolving immediate: ");
		print_tokenized_expression(stderr, imm->expression);
		fprintf(stderr, "\n");
		int error;
		uint64_t result = evaluate_expression(imm->expression, symbols, &error);
		if (error == EXPRESSION_BAD_SYMBOL) {
			add_error(errors, ERROR_UNKNOWN_SYMBOL, 0, NULL, 0, NULL);
			continue;
		} else if (error == EXPRESSION_BAD_SYNTAX) {
			add_error(errors, ERROR_INVALID_SYNTAX, 0, NULL, 0, NULL);
			continue;
		} else {
			fprintf(stderr, "Result: %04X\n", result);
			if (imm->type == IMM_TYPE_RELATIVE) {
				result += imm->base_address;
			}
			uint64_t mask = 1;
			int shift = imm->width;
			while (--shift) {
				mask <<= 1;
				mask |= 1;
			}
			if ((result & mask) != result && ~result >> imm->width) {
				add_error(errors, ERROR_VALUE_TRUNCATED, 0, NULL, 0, NULL);
			} else {
				result = result & mask;
				int j;
				for (j = 0; j < imm->width / 8; ++j) {
					area->data[imm->address] |= (result & 0xFF);
					result >>= 8;
				}
			}
		}
	}
}

void link_objects(FILE *output, list_t *objects, list_t *errors, list_t *warnings) {
	list_t *area_states = create_list();
	list_t *symbols = create_list();
	int i;
	/* TODO: Automatic relocation should take place first */
	/* Determine how big each area is and create a state for them */
	fprintf(stderr, "Creating area states\n");
	for (i = 0; i < objects->length; ++i) {
		object_t *o = objects->items[i];
		int j;
		for (j = 0; j < o->areas->length; ++j) {
			area_t *a = o->areas->items[j];
			area_state_t *as = find_area_state(area_states, a->name);
			if (as == NULL) {
				as = create_area_state(a->name);
				list_add(area_states, as);
			}
			a->final_address = as->length;
			as->length += a->data_length;
			list_add(as->areas, a);
			fprintf(stderr, "Assigning %s address %04X\n", a->name, a->final_address);
		}
	}
	/* Find a final address for all areas and relocate their symbols */
	fprintf(stderr, "Finding final addresses\n");
	uint64_t offset_addr = 0;
	for (i = 0; i < area_states->length; ++i) {
		area_state_t *as = area_states->items[i];
		as->address = offset_addr;
		offset_addr += as->length;
		int j;
		for (j = 0; j < as->areas->length; ++j) {
			area_t *a = as->areas->items[j];
			a->final_address += as->address;
			fprintf(stderr, "Relocating symbols for %s\n", a->name);
			gather_and_relocate_symbols(symbols, a, errors);
		}
	}
	/* Resolve all late immediate values */
	for (i = 0; i < area_states->length; ++i) {
		area_state_t *as = area_states->items[i];
		int j;
		for (j = 0; j < as->areas->length; ++j) {
			area_t *a = as->areas->items[j];
			resolve_immediate_values(symbols, a, errors);
		}
	}
	/* Write final output file */
	for (i = 0; i < area_states->length; ++i) {
		area_state_t *as = area_states->items[i];
		int j;
		for (j = 0; j < as->areas->length; ++j) {
			area_t *a = as->areas->items[j];
			fwrite(a->data, sizeof(uint8_t), a->data_length, output);
		}
	}
	list_free(area_states);
}
