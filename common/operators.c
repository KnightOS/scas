#include <stdint.h>
#include "operators.h"
#include "stack.h"
#include "expression.h"

uint64_t operator_add(stack_t *stack) {
	expression_token_t *left = stack_pop(stack);
	expression_token_t *right = stack_pop(stack);
	return left->number + right->number;
}

uint64_t operator_subtract(stack_t *stack) {
	expression_token_t *left = stack_pop(stack);
	expression_token_t *right = stack_pop(stack);
	return left->number - right->number;
}

uint64_t operator_multiply(stack_t *stack) {
	expression_token_t *left = stack_pop(stack);
	expression_token_t *right = stack_pop(stack);
	return left->number * right->number;
}

uint64_t operator_divide(stack_t *stack) {
	expression_token_t *left = stack_pop(stack);
	expression_token_t *right = stack_pop(stack);
	return left->number / right->number;
}
