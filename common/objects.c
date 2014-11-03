#include "objects.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

object_t *create_object() {
	object_t *o = malloc(sizeof(object_t));
	o->areas = create_list();
	return o;
}

void object_free(object_t *o) {
	list_free(o->areas);
}

area_t *create_area(const char *name) {
	area_t *a = malloc(sizeof(area_t));
	a->name = malloc(sizeof(name) + 1);
	strcpy(a->name, name);
	a->late_immediates = create_list();
	a->data_length = 0;
	a->data_capacity = 1024;
	a->data = malloc(a->data_capacity);
	return a;
}

void append_to_area(area_t *area, uint8_t *data, size_t length) {
	if ((area->data_capacity - area->data_length) < length) {
		/* Expand capacity */
		area->data = realloc(area->data, area->data_capacity + 1024);
		area->data_capacity += 1024;
	}
	memcpy(area->data + area->data_length, data, length);
	area->data_length += length;
}
