#include "linker.h"
#include "objects.h"
#include "errors.h"
#include "list.h"
#include "expression.h"
#include "instructions.h"
#include "functions.h"
#include "merge.h"
#include "runtime.h"
#include "log.h"
#include <stdint.h>
#include <stdlib.h>
#ifdef _WIN32
#include <Windows.h>
#else
#include <strings.h>
#endif

/*
 * Notes on how this could be improved:
 * A sorted list_t would make a lot of these lookups faster
 * A hashtable could also be used to handle dupes and have fast lookup
 */

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

void resolve_immediate_values(list_t *symbols, area_t *area, list_t *errors) {
	scas_log(L_DEBUG, "Resolving immediate values for area '%s' at %08X", area->name, area->final_address);
	indent_log();
	int i;
	for (i = 0; i < area->late_immediates->length; ++i) {
		late_immediate_t *imm = area->late_immediates->items[i];
		imm->instruction_address += area->final_address;
		imm->base_address += area->final_address;
		/* Temporarily add $ to symbol list */
		symbol_t sym_pc = {
			.type = SYMBOL_LABEL,
			.value = imm->instruction_address,
			.name = "$"
		};
		list_add(symbols, &sym_pc);
		int error;
		uint64_t result = evaluate_expression(imm->expression, symbols, &error);
		list_del(symbols, symbols->length - 1); // Remove $
		if (error == EXPRESSION_BAD_SYMBOL) {
			scas_log(L_ERROR, "Unable to find symbol for expression");
			add_error_from_map(errors, ERROR_UNKNOWN_SYMBOL, area->source_map, imm->instruction_address);
			continue;
		} else if (error == EXPRESSION_BAD_SYNTAX) {
			add_error_from_map(errors, ERROR_INVALID_SYNTAX, area->source_map, imm->instruction_address);
			continue;
		} else {
			if (imm->type == IMM_TYPE_RELATIVE) {
				result = result - imm->base_address;
			}
			scas_log(L_DEBUG, "Immediate value result: 0x%08X (width %d, base address 0x%08X, %02X)", result, imm->width, imm->instruction_address);
			uint64_t mask = 1;
			int shift = imm->width;
			while (--shift) {
				mask <<= 1;
				mask |= 1;
			}
			if ((result & mask) != result && ~result >> imm->width) {
				add_error_from_map(errors, ERROR_VALUE_TRUNCATED, area->source_map, imm->instruction_address);
			} else {
				result = result & mask;
				int j;
				for (j = 0; j < imm->width / 8; ++j) {
					area->data[imm->address + j] |= (result & 0xFF);
					result >>= 8;
				}
			}
		}
	}
	deindent_log();
}

void auto_relocate_area(area_t *area) {
	// Note: This is z80-specific, how should we handle that
	uint8_t rst0x8 = 0xCF;
	int i;
	for (i = 0; i < area->late_immediates->length; ++i) {
		late_immediate_t *imm = area->late_immediates->items[i];
		if (imm->base_address != imm->address && imm->type != IMM_TYPE_RELATIVE) {
			/* Relocate this */
			scas_log(L_DEBUG, "Adding relocation instruction for immediate at 0x%08X (inserting at 0x%08X)", imm->address, imm->instruction_address);
			insert_in_area(area, &rst0x8, sizeof(uint8_t), imm->instruction_address);
			++imm->address;
			/* Move everything that comes after */
			int k;
			for (k = 0; k < area->symbols->length; ++k) {
				symbol_t *sym = area->symbols->items[k];
				if (sym->type == SYMBOL_LABEL && sym->value > imm->instruction_address) {
					++sym->value;
				}
			}
			int j;
			for (j = 0; j < area->late_immediates->length; ++j) {
				late_immediate_t *_imm = area->late_immediates->items[j];
				if (_imm->base_address > imm->base_address) {
					++_imm->base_address;
					++_imm->instruction_address;
					++_imm->address;
				}
			}
		}
	}
}

void gather_symbols(list_t *symbols, area_t *area, linker_settings_t *settings) {
	int i;
	for (i = 0; i < area->symbols->length; ++i) {
		symbol_t *sym = area->symbols->items[i];
		if (find_symbol(symbols, sym->name)) {
			add_error_from_map(settings->errors, ERROR_DUPLICATE_SYMBOL,
					area->source_map, sym->defined_address);
		} else {
			list_add(symbols, sym);
		}
	}
}

void link_objects(FILE *output, list_t *objects, linker_settings_t *settings) {
	list_t *symbols = create_list(); // TODO: Use a hash table
	object_t *merged = merge_objects(objects);

	int i;
	for (i = 0; i < merged->areas->length; ++i) {
		area_t *area = merged->areas->items[i];
		gather_symbols(symbols, area, settings);
	}
	for (i = 0; i < merged->areas->length; ++i) {
		area_t *area = merged->areas->items[i];
		scas_log(L_DEBUG, "Linking area %s", area->name);
		if (scas_runtime.options.remove_unused_functions) {
			remove_unused_functions(area, merged->areas);
		}
		if (settings->automatic_relocation) {
			auto_relocate_area(area);
		}
		resolve_immediate_values(symbols, area, settings->errors);
		scas_log(L_DEBUG, "Writing final linked area to output file");
		fwrite(area->data, sizeof(uint8_t), (int)area->data_length, output);
	}
	scas_log(L_DEBUG, "Final binary written: %d bytes", ftell(output));
	list_free(symbols);
}
