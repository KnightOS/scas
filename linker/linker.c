#include "linker.h"
#include "objects.h"
#include "list.h"
#include "expression.h"
#include <stdint.h>
#include <stdlib.h>
#include <strings.h>

typedef struct {
	char *name;
	uint64_t address;
	uint64_t length;
} area_state_t;

area_state_t *find_area_state(list_t *areas, char *name) {
	int i;
	for (i = 0; i < areas->length; ++i) {
		area_state_t *a = areas->items[i];
		if (strcasecmp(a->name, name) == 0) {
			return a;
		}
	}
	return NULL;
}

void link_objects(FILE *output, list_t *objects, list_t *errors, list_t *warnings) {
	list_t *area_states = create_list();
	list_t *symbols = create_list();
	int i;

	for (i = 0; i < objects->length; ++i) {
		object_t *o = objects->items[i];
		int j;
		/* Determine how big each area is and create a state for them */
		for (j = 0; j < o->areas->length; ++j) {
			area_t *a = o->areas->items[j];
			area_state_t *as = find_area_state(area_states, a->name);
			if (as == NULL) {
				as = malloc(sizeof(area_state_t));
				as->name = a->name;
				as->address = 0; // Will be populated later on
				list_add(area_states, as);
			}
			a->final_address = as->length;
			as->length += a->data_length;
		}
	}
	/* Find a final address for all areas and relocate their symbols */
	/* Resolve all late immediate values */
	/* Write final output file */
	list_free(area_states);
}
