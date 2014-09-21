#include "expression.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"

expression_t *parse_digit(const char **string) {
	if ((*string)[1] == 'b') {
		// TODO: binary string
		return 0;
	} else {
		expression_t *expr = malloc(sizeof(expression_t));
		expr->type = NUMBER;
		char *end;
		expr->number = strtol(*string, &end, 0);
		*string = end;
		return expr;
	}
}

expression_t *parse_operator(const char **string, operator_item_t *operators, int operator_count) {
	int i;
	for (i = 0; i < operator_count; i++) {
		if (strncmp(operators[i].operator, *string, strlen(operators[i].operator)) == 0) {
			expression_t *exp = malloc(sizeof(expression_t));
			exp->type = OPERATOR;
			exp->operator = i;
			*string += strlen(operators[i].operator);
			return exp;
		}
	}

	return 0;
}

expression_t *parse_symbol(const char **string) {
	const char *end = *string;
	while (*end && !isspace(*end)) {
		end++;
	}
	char *symbol = malloc(end - *string + 1);
	strncpy(symbol, *string, end - *string);
	*string = end;

	expression_t *expr = malloc(sizeof(expression_t));
	expr->type = SYMBOL;
	expr->symbol = symbol;
	return expr;
}

// Based on shunting-yard
expression_list_t *parse_expression(const char *str, operator_item_t *operators, int operator_count) {
	char *operator_cache = malloc(operator_count + 1);
	int i;
	for (i = 0; i < operator_count; i++) {
		operator_cache[i] = operators[i].operator[0];
	}
	operator_cache[i] = 0;

	expression_list_t *list = malloc(sizeof(expression_list_t));
	list->expression_string = create_list();
	list_t *stack = create_list();

	const char *current = str;
	while (1) {
		while (isspace(*(current))) {
			current++;
		}
		expression_t *expr;
		if (*current == 0) {
			break;
		} else if (isdigit(*current)) {
			expr = parse_digit(&current);
		} else if (*current == '(') {
			expr = malloc(sizeof(expression_t));
			expr->type = OPEN_PARENTHESIS;
			list_add(stack, expr);
			current++;
			continue;
		} else if (*current == ')') {
			if (stack->length == 0) {
				// hum, nicely error here
				return 0;
			}
			expr = stack->items[stack->length - 1];
			scas_log(L_DEBUG, "Size: %d", stack->length);
			while (expr && expr->type != OPEN_PARENTHESIS) {
				scas_log(L_DEBUG, "Check expr - %p", expr);
				list_del(stack, stack->length - 1);
				list_add(list->expression_string, expr);

				if (stack->length <= 0) {
					expr = 0;
					return 0;
					break;
				}

				expr = stack->items[stack->length - 1];
			}

			list_del(stack, stack->length - 1);
			free(expr);
			current++;
			continue;
		} else if(strchr(operator_cache, *current)) {
			expr = parse_operator(&current, operators, operator_count);
		} else {
			expr = parse_symbol(&current);
		}

		if (expr->type == OPERATOR) {
			operator_item_t *operator = &operators[expr->operator];
			expression_t *expr2 = stack->length ? stack->items[stack->length - 1] : 0;
			operator_item_t *operator2 = expr2 ? &operators[expr2->operator] : 0;
			while (expr2 && expr2->type == OPERATOR && ((!operator->right_assocative && operator2->precedence == operator->precedence) || operator->precedence < operator2->precedence)) {
				list_del(stack, stack->length - 1);
				list_add(list->expression_string, expr2);
				if (!stack->length) {
					break;
				}
				expr2 = stack->items[stack->length - 1];
				operator2 = &operators[expr2->operator];
			}
			list_add(stack, expr);
		} else {
			list_add(list->expression_string, expr);
		}
	}

	while (stack->length > 0) {
		expression_t *item = stack->items[stack->length - 1];
		list_del(stack, stack->length - 1);
		list_add(list->expression_string, item);
	}

	list_free(stack);

	return list;
}
