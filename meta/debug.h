#ifndef DEBUG_H
#define DEBUG_H

#include "object.h"

typedef struct {
	uint8_t version;

	uint32_t symbol_count;
	sass_symbol_t *symbols;

	int line_count;
	sass_line_t *lines;
} sass_debug_h;

#endif
