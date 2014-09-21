#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include <stdio.h>
#include "list.h"

typedef struct {
    char *arch;
    list_t *instructions;
} instruction_set_t;

typedef struct {
    char *match;
} instruction_t;

instruction_set_t *load_instruction_set(FILE *file);
void instruction_set_free(instruction_set_t *set);

#endif
