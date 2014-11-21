#ifndef LINKER_H
#define LINKER_H
#include "list.h"
#include "objects.h"

typedef struct {
    int automatic_relocation;
    int merge_only;
    list_t *errors;
    list_t *warnings;
} linker_settings_t;

void link_objects(FILE *output, list_t *objects, linker_settings_t *settings);

#endif
