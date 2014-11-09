#ifndef LINKER_H
#define LINKER_H
#include "list.h"
#include "objects.h"

void link_objects(FILE *output, list_t *objects, list_t *errors, list_t *warnings);

#endif
