#include <stdint.h>
#include <stdlib.h>
#include "operators.h"
#include "stack.h"
#include "expression.h"

uint64_t operator_add(stack_t *stack, int *error) {
	expression_token_t *left = stack_pop(stack);
	expression_token_t *right = stack_pop(stack);
	if (left == NULL || right == NULL) {
		*error = EXPRESSION_BAD_SYNTAX;
		return 0;
	}
	return left->number + right->number;
}

uint64_t operator_subtract(stack_t *stack, int *error) {
	expression_token_t *left = stack_pop(stack);
	expression_token_t *right = stack_pop(stack);
	if (left == NULL || right == NULL) {
		*error = EXPRESSION_BAD_SYNTAX;
		return 0;
	}
	return left->number - right->number;
}

uint64_t operator_multiply(stack_t *stack, int *error) {
	expression_token_t *left = stack_pop(stack);
	expression_token_t *right = stack_pop(stack);
	if (left == NULL || right == NULL) {
		*error = EXPRESSION_BAD_SYNTAX;
		return 0;
	}
	return left->number * right->number;
}

uint64_t operator_divide(stack_t *stack, int *error) {
	expression_token_t *left = stack_pop(stack);
	expression_token_t *right = stack_pop(stack);
	if (left == NULL || right == NULL) {
		*error = EXPRESSION_BAD_SYNTAX;
		return 0;
	}
	return left->number / right->number;
}
