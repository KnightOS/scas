#ifndef EXPRESSION_H
#define EXPRESSION_H

#include "list.h"

#include <stdint.h>

enum {
	SYMBOL,
	NUMBER,
	OPERATOR,
	OPEN_PARENTHESIS,
};

typedef struct expression_list expression_list_t;

typedef struct {
	int type;
	union {
		char *symbol;
		int32_t number;
		int operator;
	};
} expression_t;

struct expression_list {
	list_t *expression_string;
};

typedef struct {
	char *operator;
	int precedence;
	int right_assocative;
	void (*function)(expression_list_t *);
} operator_item_t;

expression_list_t *parse_expression(const char *str, operator_item_t *operators, int operator_count);

#endif
