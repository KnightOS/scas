#include "objects.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

object_t *create_object() {
	object_t *o = malloc(sizeof(object_t));
	o->areas = create_list();
	o->defined_symbols = create_list();
	o->required_symbols = create_list();
	return o;
}

void object_free(object_t *o) {
	list_free(o->areas);
	list_free(o->defined_symbols);
	list_free(o->required_symbols);
}

area_t *create_area(const char *name) {
	area_t *a = malloc(sizeof(area_t));
	a->name = malloc(sizeof(name) + 1);
	strcpy(a->name, name);
	a->late_immediates = create_list();
	a->data_length = 1024;
	a->data = malloc(a->data_length);
	return a;
}
