#ifndef ASSEMBLE_H
#define ASSEMBLE_H

#include "object.h"

typedef struct {
	FILE **files;
	int files_len;
} assembly_input_t;

object_t assemble(assembly_input_t input);

#endif
