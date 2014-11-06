#include "stack.h"
#include "stdlib.h"

stack_t *create_stack() {
	stack_t *stack = malloc(sizeof *stack);

	stack->capacity = STACK_GROWTH_RATE;
	stack->length = 0;
	stack->items = calloc(STACK_GROWTH_RATE, sizeof(void*));

	return stack;
}

void stack_free(stack_t *stack) {
	free(stack->items);
	free(stack);
}

void stack_push(stack_t *stack, void *item) {
	if (stack->capacity <= stack->length) {
		stack->capacity += STACK_GROWTH_RATE;
		stack->items = realloc(stack->items, stack->capacity);
	}

	stack->items[stack->length++] = item;
}

void *stack_pop(stack_t *stack) {
	if (stack->length == 0) {
		return NULL;
	}
	return stack->items[--stack->length];
}

void *stack_peek(stack_t *stack) {
	return stack->items[stack->length - 1];
}

void stack_shrink_to_fit(stack_t *stack) {
	stack->capacity = stack->length;
	stack->items = realloc(stack->items, stack->capacity);
}
