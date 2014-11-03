#ifndef STACK_H
#define STACK_H

typedef struct {
    int capacity;
    int length;
    void **items;
} stack_t;

stack_t *create_stack();
void stack_free(stack_t *stack);
void stack_push(stack_t *stack, void *item);
void *stack_pop(list_t *stack);

#endif
