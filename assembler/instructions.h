#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include <stdio.h>
#include <stdint.h>
#include "list.h"

typedef struct {
    char *arch;
    list_t *instructions;
    list_t *operand_groups;
} instruction_set_t;

typedef struct {
    char *match;
    uint64_t value;
    int length;
    list_t *immediate;
    list_t *operands;
} instruction_t;

typedef struct {
    char ref;
    int length;
    int shift;
    char *value;
} immediate_t;

typedef struct {
    char *name;
    list_t *operands;
} operand_group_t;

typedef struct {
    char *match;
    uint64_t value;
} operand_t;

typedef struct {
    char ref;
    operand_t operand;
} operand_ref_t;

instruction_set_t *load_instruction_set(FILE *file);
void instruction_set_free(instruction_set_t *set);
instruction_t *match_instruction(const char *text);

#endif
