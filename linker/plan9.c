#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include "list.h"
#include "stack.h"
#include "expression.h"
#include "objects.h"
#include "linker.h"
#include "plan9.h"
#include "instructions.h"
#include "runtime.h"
#include "log.h"
#include "errors.h"
#include "merge.h"

#define HDR_MAGIC	0x00008000		/* header expansion */

#define	_MAGIC(f, b)	((f)|((((4*(b))+0)*(b))+7))
#define	A_MAGIC		_MAGIC(0, 8)		/* 68020 */
#define	I_MAGIC		_MAGIC(0, 11)		/* intel 386 */
#define	J_MAGIC		_MAGIC(0, 12)		/* intel 960 (retired) */
#define	K_MAGIC		_MAGIC(0, 13)		/* sparc */
#define	V_MAGIC		_MAGIC(0, 16)		/* mips 3000 BE */
#define	X_MAGIC		_MAGIC(0, 17)		/* att dsp 3210 (retired) */
#define	M_MAGIC		_MAGIC(0, 18)		/* mips 4000 BE */
#define	D_MAGIC		_MAGIC(0, 19)		/* amd 29000 (retired) */
#define	E_MAGIC		_MAGIC(0, 20)		/* arm */
#define	Q_MAGIC		_MAGIC(0, 21)		/* powerpc */
#define	N_MAGIC		_MAGIC(0, 22)		/* mips 4000 LE */
#define	L_MAGIC		_MAGIC(0, 23)		/* dec alpha (retired) */
#define	P_MAGIC		_MAGIC(0, 24)		/* mips 3000 LE */
#define	U_MAGIC		_MAGIC(0, 25)		/* sparc64 */
#define	S_MAGIC		_MAGIC(HDR_MAGIC, 26)	/* amd64 */
#define	T_MAGIC		_MAGIC(HDR_MAGIC, 27)	/* powerpc64 */
#define	R_MAGIC		_MAGIC(HDR_MAGIC, 28)	/* arm64 */

void write_be32(FILE *f, uint32_t value) {
	uint8_t bytes[4];
	bytes[0] = (value >> 24) & 0xff;
	bytes[1] = (value >> 16) & 0xff;
	bytes[2] = (value >> 8) & 0xff;
	bytes[3] = (value) & 0xff;
	fwrite(bytes, 4, 1, f);
}
void write_be64(FILE *f, uint64_t value) {
	write_be32(f, (value >> 32) & 0xffffffff);
	write_be32(f, (value) & 0xffffffff);
}

// returns 1 on failure
uint32_t get_magic(char* arch) {
	if (strcmp(arch, "amd64")) {
		return S_MAGIC;
	}
	if (strcmp(arch, "arm64")) {
		return R_MAGIC;
	}
	return 1;
}
uint8_t get_symtype_from_area(area_t* area) {
	if (strcmp(area->name, "_CODE") == 0) {
		return 't';
	} else if (strcmp(area->name, "_DATA") == 0) {
		return 'd';
	} else {
		scas_log(L_ERROR, "unknown area type for plan9 %s", area->name);
		return 0;
	}
}

int output_plan9(FILE *f, object_t *object, linker_settings_t *settings) {
	area_t* data = 0;
	area_t* text = 0;
	uint32_t bss_len = 0;
	uint32_t syms_len = 0;
	uint64_t entry = 0;

	for (unsigned int i = 0; i < object->areas->length; i++){
		area_t* area = object->areas->items[i];
		if (strcmp("_CODE", area->name) == 0) {
			text = area;
			relocate_area(text, 0x200028, true);
		} else if (strcmp("_DATA", area->name) == 0) {
			data = area;
			relocate_area(data, 0x400000, true);
		} else {
			scas_log(L_ERROR, "unknown section name for plan9 a.out format: %s", area->name);
			return 1;
		}
	}
	if (text == 0) {
		scas_log(L_ERROR, "plan9 a.out format needs text section");
		return 1;
	}

	// the header
	uint32_t magic = get_magic(scas_runtime.arch);
	bool is64 = (magic & HDR_MAGIC) >> 15 == 1;
	if (magic == 1) return 1;
	write_be32(f, magic);
	write_be32(f, text->data_length);
	if (data != 0)
		write_be32(f, data->data_length);
	else
		write_be32(f, 0);
	write_be32(f, bss_len);
	write_be32(f, syms_len);
	write_be32(f, entry);
	write_be32(f, 0); // size of pc/sp offset table
	write_be32(f, 0); // size of pc/line number table
	// fat header
	if (is64)
		write_be64(f, entry);

	// write the text
	fwrite(text->data, 1, text->data_length, f);
	// write the data (if there is any)
	if (data != 0)
		fwrite(data->data, 1, data->data_length, f);

	list_t *symbols = create_list();
	char type;
	for(unsigned int i = 0; i < object->areas->length; i += 1) {
		area_t* area = object->areas->items[i];
		type = get_symtype_from_area(area);
		for(unsigned int i = 0; i < area->symbols->length; ++i){
			symbol_t *s = area->symbols->items[i];
			if(find_symbol(symbols, s->name))
				add_error_from_map(settings->errors, ERROR_DUPLICATE_SYMBOL,
								   area->source_map, s->defined_address, s->name);
			else {
				if (is64)
					write_be64(f, s->value);
				else
					write_be32(f, s->value);
				uint8_t type_toput = (s->exported ? toupper(type) : type) | 0x80;
				fputc(type_toput, f);
				int namelen = strlen(s->name) + 1;
				fwrite(s->name, sizeof(char), namelen, f);
				if (strcmp("start", s->name) == 0)
					entry = s->value;
				syms_len += 8 + 1 + namelen;
			}
		}
	}

	// write back into the header
	// write the symbol table len
	fseek(f, 4 * 4, SEEK_SET);
	write_be32(f, syms_len);
	// entry point
	fseek(f, 5 * 4, SEEK_SET);
	write_be32(f, entry);
	// fat entry point
	if (is64) {
		fseek(f, 8 * 4, SEEK_SET);
		write_be64(f, entry);
	}

	return 0;
}
