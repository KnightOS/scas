#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "list.h"
#include "stack.h"
#include "expression.h"
#include "objects.h"
#include "linker.h"
#include "8xp.h"
#include "instructions.h"
#include "runtime.h"

#define UNPROTECTED_8XP 0x05
#define PROTECTED_8XP 0x06

int write_le16(FILE *f, uint16_t value) {
	uint8_t bytes[2];
	bytes[0] = value & 0xff;
	bytes[1] = value >> 8;
	return fwrite(bytes, 2, 1, f);
}

void write_8xp_header(FILE *f, int data_len) {
	const char *magic = "**TI83F*\x1A\x0A";
	const char *comment = "Generated by scas";

	/* Magic number */
	fwrite(magic, sizeof(char), strlen(magic) + 1, f);

	/* Comment */
	fwrite(comment, sizeof(char), strlen(comment), f);
	for (size_t i = 0; i < 42 - strlen(comment); ++i) {
		fputc(' ', f);
	}

	/* Length of variable section */
	uint16_t n = data_len + 19;
	write_le16(f, n);
}

void write_8xp_data(FILE *f, uint8_t *data, int len) {
	(void)data;
	uint16_t const_1 = 0x000D;
	write_le16(f, const_1);

	/* Data length */
	uint16_t dlen = len + 2;
	write_le16(f, dlen);

	/* Variable type */
	uint8_t type = PROTECTED_8XP;
	if (!scas_runtime.options.prog_protected_8xp) {
		type = UNPROTECTED_8XP;
	}
	fwrite(&type, sizeof(uint8_t), 1, f);

	/* Variable name */
	fwrite(scas_runtime.options.prog_name_8xp, sizeof(char),
		strlen(scas_runtime.options.prog_name_8xp), f);
	for (size_t i = 0; i < 8 - strlen(scas_runtime.options.prog_name_8xp); ++i) {
		fputc('\0', f); /* Pad to 8 chars */
	}

	/* Version (constant) */
	uint8_t ver = 0;
	fwrite(&ver, sizeof(uint8_t), 1, f);

	/* Archive flag */
	uint8_t archived = 0;
	if (scas_runtime.options.prog_archived_8xp) {
		archived = 0x80;
	}
	fwrite(&archived, sizeof(uint8_t), 1, f);

	/* Data length (again) */
	write_le16(f, dlen);

	/* Data length (in 8xp data) */
	dlen -= 2;
	write_le16(f, dlen);
}

int output_8xp(FILE *f, object_t *object, linker_settings_t *settings) {
	int a;
	long t;
	uint16_t sum = 0;
	area_t *final = areas_merge(object->areas, settings->errors);
	if(final == NULL)
		return 1;
	write_8xp_header(f, final->data_length);
	t = ftell(f);
	write_8xp_data(f, final->data, final->data_length);
	fwrite(final->data, sizeof(uint8_t), final->data_length, f);
	/* Calculate checksum */
	fseek(f, t, SEEK_SET);
	while((a = fgetc(f)) != EOF)
		sum += a;
	write_le16(f, sum);
	return 0;
}
