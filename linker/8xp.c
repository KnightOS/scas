#include "8xp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "runtime.h"

#define UNPROTECTED_8XP 0x05
#define PROTECTED_8XP 0x06

void write_8xp_header(FILE *f, int data_len) {
	const char *magic = "**TI83F*\x1A\x0A";
	const char *comment = "scas z80 assembler";

	/* Magic number */
	fwrite(magic, sizeof(char), strlen(magic) + 1, f);

	/* Comment */
	fwrite(comment, sizeof(char), strlen(comment), f);
	int i;
	for (i = 0; i < 42 - strlen(comment); ++i) {
		fputc(' ', f);
	}

	/* Length of variable section */
	uint16_t n = data_len + 19;
	fwrite(&n, sizeof(uint16_t), 1, f);
}

void write_8xp_data(FILE *f, uint8_t *data, int len) {
	uint16_t const_1 = 0x000D;
	fwrite(&const_1, sizeof(uint16_t), 1, f);

	/* Data length */
	uint16_t dlen = len + 2;
	fwrite(&dlen, sizeof(uint16_t), 1, f);

	/* Variable type */
	uint8_t type = PROTECTED_8XP;
	if (!scas_runtime.options.prog_protected_8xp) {
		type = UNPROTECTED_8XP;
	}
	fwrite(&type, sizeof(uint8_t), 1, f);

	/* Variable name */
	fwrite(scas_runtime.options.prog_name_8xp, sizeof(char),
		strlen(scas_runtime.options.prog_name_8xp), f);
	int i;
	for (i = 0; i < 8 - strlen(scas_runtime.options.prog_name_8xp); ++i) {
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
	fwrite(&dlen, sizeof(uint16_t), 1, f);

	/* Data length (in 8xp data) */
	dlen -= 2;
	fwrite(&dlen, sizeof(uint16_t), 1, f);
}

int output_8xp(FILE *f, uint8_t *data, int data_length) {
	write_8xp_header(f, data_length);
	long t = ftell(f);
	write_8xp_data(f, data, data_length);
	fwrite(data, sizeof(uint8_t), data_length, f);
	/* Calculate checksum */
	uint16_t sum = 0;
	fseek(f, t, SEEK_SET);
	int a;
	while ((a = fgetc(f)) != EOF) {
		sum += a;
	}
	fwrite(&sum, sizeof(uint16_t), 1, f);
	return 0;
}