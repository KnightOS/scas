#ifndef MERGE_H
#define MERGE_H
#include "list.h"
#include "objects.h"
#include "linker.h"

void merge_objects(FILE *output, list_t *objects, linker_settings_t *settings);

#endif
