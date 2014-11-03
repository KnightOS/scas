#ifndef OBJECT_H
#define OBJECT_H
#include "list.h"
#include <stdint.h>
#include <stddef.h>

typedef struct {
    char *expression;
    uint64_t address;
} late_immediate_t;

typedef struct {
    char *name;
    list_t *late_immediates;
    uint8_t *data;
    int data_length;
    int data_capacity;
} area_t;

typedef struct {
    list_t *areas;
    list_t *defined_symbols;
    list_t *required_symbols;
} object_t;

object_t *create_object();
void object_free(object_t *object);
area_t *create_area(const char *name);
void append_to_area(area_t *area, uint8_t *data, size_t length);

#endif
