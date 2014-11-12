#include "linker.h"
#include "objects.h"
#include "errors.h"
#include "list.h"
#include "expression.h"
#include "instructions.h"
#include "log.h"
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

void gather_and_relocate_symbols(list_t *symbols, area_t *area, list_t *errors) {
	scas_log(L_DEBUG, "Relocating symbols for area '%s' (at 0x%08X)", area->name, area->final_address);
	indent_log();
	int i;
	for (i = 0; i < area->symbols->length; ++i) {
		symbol_t *sym = area->symbols->items[i];
		symbol_t *existing = find_symbol(symbols, sym->name);
		if (existing) {
			add_error_from_map(errors, ERROR_DUPLICATE_SYMBOL, area->source_map, ~0);
			continue;
		}
		if (sym->type == SYMBOL_LABEL) {
			sym->value += area->final_address;
			scas_log(L_DEBUG, "Assigned symbol '%s' value 0x%08X", sym->name, sym->value);
			list_add(symbols, sym);
		}
	}
	deindent_log();
}

void resolve_immediate_values(list_t *symbols, area_t *area, list_t *errors) {
	scas_log(L_DEBUG, "Resolving immediate values for area '%s'", area->name);
	indent_log();
	int i;
	for (i = 0; i < area->late_immediates->length; ++i) {
		late_immediate_t *imm = area->late_immediates->items[i];
		scas_log(L_DEBUG, "Resolving immediate value @0x%08X", imm->address);
		int error;
		uint64_t result = evaluate_expression(imm->expression, symbols, &error);
		if (error == EXPRESSION_BAD_SYMBOL) {
			add_error_from_map(errors, ERROR_UNKNOWN_SYMBOL, area->source_map, imm->base_address);
			continue;
		} else if (error == EXPRESSION_BAD_SYNTAX) {
			add_error_from_map(errors, ERROR_INVALID_SYNTAX, area->source_map, imm->base_address);
			continue;
		} else {
			if (imm->type == IMM_TYPE_RELATIVE) {
				result += imm->base_address;
			}
			scas_log(L_DEBUG, "Immediate value result: 0x%08X (width %d, base address 0x%08X)", result, imm->width, imm->base_address);
			uint64_t mask = 1;
			int shift = imm->width;
			while (--shift) {
				mask <<= 1;
				mask |= 1;
			}
			if ((result & mask) != result && ~result >> imm->width) {
				add_error_from_map(errors, ERROR_VALUE_TRUNCATED, area->source_map, imm->base_address);
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
	deindent_log();
}

void link_objects(FILE *output, list_t *objects, list_t *errors, list_t *warnings) {
	scas_log(L_INFO, "Linking %d objects together", objects->length);
	list_t *area_states = create_list();
	list_t *symbols = create_list();
	int i;
	/* TODO: Automatic relocation should take place first */
	/* Determine how big each area is and create a state for them */
	scas_log(L_DEBUG, "Assigning addresses for each area");
	indent_log();
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
			scas_log(L_DEBUG, "Added %d bytes to section %s, total %d bytes", a->data_length, as->name, as->length);
			list_add(as->areas, a);
			scas_log(L_DEBUG, "Assigned address 0x%08X to area '%s' from object %d",
					(unsigned long)a->final_address, a->name, i);
		}
	}
	deindent_log();
	/* Find a final address for all areas and relocate their symbols */
	scas_log(L_DEBUG, "Assigning final address to each area in final executable");
	indent_log();
	uint64_t offset_addr = 0;
	for (i = 0; i < area_states->length; ++i) {
		area_state_t *as = area_states->items[i];
		as->address = offset_addr;
		offset_addr += as->length;
		scas_log(L_DEBUG, "Assigned area %s to address %08X (%d bytes)", as->name, as->address, as->length);
		int j;
		for (j = 0; j < as->areas->length; ++j) {
			area_t *a = as->areas->items[j];
			a->final_address += as->address;
			scas_log(L_DEBUG, "Assigned final address 0x%08X for area '%s:%d'", a->final_address, a->name, j);
			gather_and_relocate_symbols(symbols, a, errors);
		}
	}
	deindent_log();
	/* Resolve all late immediate values */
	scas_log(L_DEBUG, "Resolving immediate values in each area");
	indent_log();
	for (i = 0; i < area_states->length; ++i) {
		area_state_t *as = area_states->items[i];
		int j;
		for (j = 0; j < as->areas->length; ++j) {
			area_t *a = as->areas->items[j];
			resolve_immediate_values(symbols, a, errors);
		}
	}
	deindent_log();
	scas_log(L_DEBUG, "Writing final linked output file");
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
