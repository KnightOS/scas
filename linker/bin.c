#include <stdio.h>
#include <stdint.h>
#include <stdbool.h> 
#include "list.h"
#include "stack.h"
#include "expression.h"
#include "objects.h"
#include "linker.h"
#include "bin.h"

int
output_bin(FILE *f, object_t *obj, linker_settings_t *settings)
{
	area_t *final = areas_merge(obj->areas, settings->errors);
	int ret = fwrite(final->data, sizeof(uint8_t), final->data_length, f) != final->data_length;
	merged_area_free(final);
	return ret;
}
