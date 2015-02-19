#ifndef MERGE_H
#define MERGE_H
#include "list.h"
#include "objects.h"
#include "linker.h"

object_t *merge_objects(list_t *objects);
void merge_areas(object_t *merged, object_t *source);

#endif
