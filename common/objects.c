#include "objects.h"
#include <stdlib.h>

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
