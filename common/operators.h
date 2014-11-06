#ifndef OPERATORS_H
#define OPERATORS_H
#include <stdint.h>
#include "stack.h"

enum {
    OP_PLUS = 1,
    OP_MINUS = 2,
    OP_MULTIPLY = 4,
    OP_DIVIDE = 3,
};

uint64_t operator_add(stack_t *stack, int *error);
uint64_t operator_subtract(stack_t *stack, int *error);
uint64_t operator_multiply(stack_t *stack, int *error);
uint64_t operator_divide(stack_t *stack, int *error);

#endif
