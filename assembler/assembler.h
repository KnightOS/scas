#ifndef ASSEMBLER_H
#define ASSEMBLER_H
#include <stdio.h>
#include "list.h"
#include "objects.h"
#include "instructions.h"

object_t *assemble(FILE *file, instruction_set_t *set);

#endif
