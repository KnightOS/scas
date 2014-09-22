#ifndef EXPRESSION_H
#define EXPRESSION_H

#include "list.h"

#include <stdint.h>

enum {
	SYMBOL = 0x00,
	NUMBER = 0x01,
	OPERATOR = 0x02,
	OPEN_PARENTHESIS,
};

typedef struct {
	int type;
	union {
		char *symbol;
		int32_t number;
		int operator;
	};
} expression_t;

typedef struct {
	list_t *expression_string;
	list_t *used_symbols;
} expression_list_t;

typedef struct {
	char *operator;
	int precedence;
	int right_assocative;
	void (*function)(expression_list_t *);
} operator_item_t;


// NOTE: when passing operators, pass them sorted by length, largest first.
// Otherwise the parser may parse e.g. '>>' as '>' '>'.
expression_list_t *parse_expression(const char *str, operator_item_t *operators, int operator_count);

#endif
