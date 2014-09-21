#ifndef EXPRESSION_H
#define EXPRESSION_H

#include "list.h"

enum {
	SYMBOL,
	NUMBER,
	OPERATOR
};

typedef struct expression_list expression_list_t;

typedef struct {
	int type;
	union {
		struct {
			char *symbol;
		} symbol;

		int32_t number;
		void (*operator)(expression_list_t);
	};
} expresion_t;

struct expression_list {
	list_t *expression_string;
};

expression_list_t *parse_expression(const char *str);

#endif
