#include "objects.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

object_t *create_object() {
	object_t *o = malloc(sizeof(object_t));
	o->areas = create_list();
	return o;
}

void object_free(object_t *o) {
	list_free(o->areas);
}

area_t *create_area(const char *name) {
	area_t *a = malloc(sizeof(area_t));
	a->name = malloc(sizeof(name) + 1);
	strcpy(a->name, name);
	a->late_immediates = create_list();
	a->symbols = create_list();
	a->data_length = 0;
	a->data_capacity = 1024;
	a->data = malloc(a->data_capacity);
	return a;
}

void append_to_area(area_t *area, uint8_t *data, size_t length) {
	if ((area->data_capacity - area->data_length) < length) {
		/* Expand capacity */
		area->data = realloc(area->data, area->data_capacity + 1024);
		area->data_capacity += 1024;
	}
	memcpy(area->data + area->data_length, data, length);
	area->data_length += length;
}

void write_area(FILE *f, area_t *a) {
	uint32_t len;
	int i;
	fprintf(f, "%s", a->name); fputc(0, f);
	/* Symbols */
	len = a->symbols->length;
	fwrite(&len, sizeof(uint32_t), 1, f);
	for (i = 0; i < a->symbols->length; ++i) {
		symbol_t *sym = a->symbols->items[i];
		fputc(sym->exported, f);
		len = strlen(sym->name);
		fwrite(&len, sizeof(uint32_t), 1, f);
		fprintf(f, "%s", sym->name);
		fwrite(&sym->value, sizeof(uint64_t), 1, f);
		fwrite(&sym->defined_address, sizeof(uint64_t), 1, f);
	}
	/* Expressions */
	len = a->late_immediates->length;
	fwrite(&len, sizeof(uint32_t), 1, f);
	for (i = 0; i < a->late_immediates->length; ++i) {
		late_immediate_t *imm = a->late_immediates->items[i];
		fputc(imm->type, f);
		fputc(imm->width, f);
		fwrite(&imm->address, sizeof(uint64_t), 1, f); /* TODO: Instruction address */
		fwrite(&imm->address, sizeof(uint64_t), 1, f);
		fwrite_tokens(f, imm->expression);
	}
	/* Machine code */
	len = a->data_length;
	fwrite(&len, sizeof(uint64_t), 1, f);
	fwrite(a->data, sizeof(uint8_t), a->data_length, f);
	/* Source map TODO */
	len = 0;
	fwrite(&len, sizeof(uint64_t), 1, f);
}

void fwriteobj(FILE *f, object_t *o, char *arch) {
	/* Header */
	fprintf(f, "SCASOBJ\x01%s", arch); fputc(0, f);
	/* Areas */
	uint32_t a_len = o->areas->length;
	fwrite(&a_len, sizeof(uint32_t), 1, f);
	int i;
	for (i = 0; i < o->areas->length; ++i) {
		area_t *a = o->areas->items[i];
		write_area(f, a);
	}
	fflush(f);
}
