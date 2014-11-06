#ifndef EXPRESSION_H
#define EXPRESSION_H
#include "list.h"
#include "stack.h"
#include <stdint.h>

enum {
	SYMBOL = 0x00,
	NUMBER = 0x01,
	OPERATOR = 0x02,
	OPEN_PARENTHESIS
};

enum {
	EXPRESSION_BAD_SYMBOL = 1,
	EXPRESSION_BAD_SYNTAX = 2
};

typedef struct {
	int type;
	char *symbol;
	uint64_t number;
	int operator;
} expression_token_t;

typedef struct {
	list_t *tokens;
	list_t *symbols;
} tokenized_expression_t;

typedef struct {
	char *operator;
	int operator_type;
	int precedence;
	int right_assocative;
	uint64_t (*function)(stack_t *, int *);
} operator_t;

// NOTE: when passing operators, pass them sorted by length, largest first.
// Otherwise the parser may parse e.g. '>>' as '>' '>'.
tokenized_expression_t *parse_expression(const char *str);
void initialize_expressions();
void print_tokenized_expression(tokenized_expression_t *expression);
uint64_t evaluate_expression(tokenized_expression_t *expression, list_t *symbols, int *error);

#endif
