#ifndef OBJECT_H
#define OBJECT_H
#include "list.h"
#include "expression.h"
#include <stdint.h>
#include <stdio.h>
#include <stddef.h>

enum {
    SYMBOL_LABEL,
    SYMBOL_EQUATE
};

typedef struct {
    int type;
    char *name;
    uint64_t value;
    uint64_t defined_address;
    int exported;
} symbol_t;

typedef struct {
    tokenized_expression_t *expression;
    uint64_t width;
    uint64_t address;
    uint64_t base_address;
    int type;
} late_immediate_t;

typedef struct {
    uint64_t line_number;
    uint64_t address;
    uint64_t length;
    char *source_code;
} source_map_entry_t;

typedef struct {
    char *file_name;
    list_t *entries;
} source_map_t;

typedef struct {
    char *name;
    list_t *late_immediates;
    list_t *symbols;
    list_t *source_map;
    uint8_t *data;
    uint64_t data_length;
    uint64_t data_capacity;
} area_t;

typedef struct {
    list_t *areas;
} object_t;

object_t *create_object();
void object_free(object_t *object);
area_t *create_area(const char *name);
void append_to_area(area_t *area, uint8_t *data, size_t length);
void fwriteobj(FILE *file, object_t *object, char *arch);
void add_source_map(source_map_t *map, int line_number, const char *line, uint64_t address, uint64_t length);
source_map_t *create_source_map(area_t *area, const char *file_name);

#endif
