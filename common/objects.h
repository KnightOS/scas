#ifndef OBJECT_H
#define OBJECT_H
#include "list.h"

typedef struct {
    list_t *areas;
    list_t *defined_symbols;
    list_t *required_symbols;
} object_t;

object_t *create_object();
void object_free(object_t *object);

#endif
