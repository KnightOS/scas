#include "objects.h"
#include "log.h"
#include "readline.h"
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
	a->source_map = create_list();
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
	scas_log(L_DEBUG, "Added %d bytes to area '%s' (now %d bytes total)", length, area->name, area->data_length);
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
	/* Imports (TODO) */
	len = 0;
	fwrite(&len, sizeof(uint32_t), 1, f);
	/* Expressions */
	len = a->late_immediates->length;
	fwrite(&len, sizeof(uint32_t), 1, f);
	for (i = 0; i < a->late_immediates->length; ++i) {
		late_immediate_t *imm = a->late_immediates->items[i];
		fputc(imm->type, f);
		fputc(imm->width, f);
		fwrite(&imm->base_address, sizeof(uint64_t), 1, f);
		fwrite(&imm->address, sizeof(uint64_t), 1, f);
		fwrite_tokens(f, imm->expression);
	}
	/* Machine code */
	len = a->data_length;
	fwrite(&len, sizeof(uint64_t), 1, f);
	fwrite(a->data, sizeof(uint8_t), a->data_length, f);
	/* Source map */
	len = a->source_map->length;
	fwrite(&len, sizeof(uint64_t), 1, f);
	for (i = 0; i < a->source_map->length; ++i) {
		source_map_t *map = a->source_map->items[i];
		fwrite(map->file_name, sizeof(char), strlen(map->file_name), f);
		len = map->entries->length;
		fwrite(&len, sizeof(uint64_t), 1, f);
		int j;
		for (j = 0; j < map->entries->length; ++j) {
			source_map_entry_t *entry = map->entries->items[j];
			len = entry->line_number;
			fwrite(&len, sizeof(uint64_t), 1, f);
			len = entry->address;
			fwrite(&len, sizeof(uint64_t), 1, f);
			len = entry->length;
			fwrite(&len, sizeof(uint64_t), 1, f);
			fwrite(entry->source_code, sizeof(char), strlen(entry->source_code), f);
		}
	}
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

area_t *read_area(FILE *f) {
	char *name = read_line(f);
	area_t *area = create_area(name);
	uint32_t symbols, imports, immediates;
	fread(&symbols, sizeof(uint32_t), 1, f);
	// TODO: Read more stuff
	return area;
}

object_t *freadobj(FILE *f, const char *name) {
	object_t  *o = create_object();
	char magic[7];
	int len = fread(magic, sizeof(char), 7, f);
	if (len != 7 || strncmp("SCASOBJ", magic, 7) != 0) {
		scas_abort("'%s' is not a valid object file.", name);
	}
	int ver = fgetc(f);
	if (ver != 1) {
		scas_abort("'%s' was built with an incompatible version of scas.", name);
	}
	uint32_t area_count;
	fread(&area_count, sizeof(uint32_t), 1, f);
	int i;
	for (i = 0; i < area_count; ++i) {
		list_add(o->areas, read_area(f));
	}
	return o;
}

source_map_t *create_source_map(area_t *area, const char *file_name) {
	source_map_t *map = malloc(sizeof(source_map_t));
	map->file_name = malloc(strlen(file_name));
	strcpy(map->file_name, file_name);
	map->entries = create_list();
	list_add(area->source_map, map);
	return map;
}

void add_source_map(source_map_t *map, int line_number, const char *line, uint64_t address, uint64_t length) {
	source_map_entry_t *entry = malloc(sizeof(source_map_entry_t));
	entry->line_number = line_number;
	entry->address = address;
	entry->length = length;
	entry->source_code = malloc(strlen(line) + 1);
	strcpy(entry->source_code, line);
	list_add(map->entries, entry);
}
