#include "expression.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "log.h"
#include "stack.h"
#include "operators.h"

static operator_t operators[] = {
	{ "+", OP_PLUS, 0, 0, operator_add },
	{ "-", OP_MINUS, 0, 0, operator_subtract },
	{ "*", OP_MULTIPLY, 0, 0, operator_multiply },
	{ "/", OP_DIVIDE, 0, 0, operator_divide }
};

void print_tokenized_expression(tokenized_expression_t *expression) {
	int i;
	for (i = 0; i < expression->tokens->length; ++i) {
		expression_token_t *token = expression->tokens->items[i];
		switch (token->type) {
			case SYMBOL:
				printf("%s ", token->symbol);
				break;
			case NUMBER:
				printf("0x%04X ", (unsigned int)token->number);
				break;
			case OPERATOR:
				printf("%s ", operators[token->operator].operator);
				break;
		}
	}
}

uint64_t evaluate_expression(tokenized_expression_t *expression, list_t symbols, int *error) {
	stack_t *stack = create_stack();
	expression_token_t *token;
	uint64_t res = 0;
	*error = 0;

	/* TODO */

	if (stack->length == 0) {
		*error = 1;
	} else {
		token = stack_pop(stack);
		if (token->type != NUMBER) {
			*error = 1;
		} else {
			res = token->number;
		}
	}
	stack_free(stack);
	return res;
}

expression_token_t *parse_digit(const char **string) {
	if (strlen(*string) > 1 && (*string)[1] == 'b') {
		// TODO: binary string
		return 0;
	} else {
		expression_token_t *expr = malloc(sizeof(expression_token_t));
		expr->type = NUMBER;
		char *end;
		expr->number = strtol(*string, &end, 0);
		*string = end;
		return expr;
	}
}

expression_token_t *parse_operator(const char **string) {
	int i;
	for (i = 0; i < sizeof(operators) / sizeof(operator_t); i++) {
		operator_t op = operators[i];
		if (strncmp(op.operator, *string, strlen(op.operator)) == 0) {
			expression_token_t *exp = malloc(sizeof(expression_token_t));
			exp->type = OPERATOR;
			exp->operator = i;
			*string += strlen(op.operator);
			return exp;
		}
	}

	return 0;
}

expression_token_t *parse_symbol(const char **string) {
	const char *end = *string;
	while (*end && isalnum(*end)) {
		end++;
	}
	char *symbol = malloc(end - *string + 1);
	strncpy(symbol, *string, end - *string);
	symbol[end - *string] = '\0';
	*string = end;

	expression_token_t *expr = malloc(sizeof(expression_token_t));
	expr->type = SYMBOL;
	expr->symbol = symbol;
	return expr;
}

// Based on shunting-yard
tokenized_expression_t *parse_expression(const char *str) {
	char *operator_cache = malloc(sizeof(operators) / sizeof(operator_t) + 1);
	int i;
	for (i = 0; i < sizeof(operators) / sizeof(operator_t); i++) {
		operator_t op = operators[i];
		operator_cache[i] = op.operator[0];
	}
	operator_cache[i] = 0;

	tokenized_expression_t *list = malloc(sizeof(tokenized_expression_t));
	list->tokens = create_list();
	list->symbols = create_list();
	stack_t *stack = create_stack();

	const char *current = str;
	while (1) {
		while (isspace(*(current))) {
			current++;
		}
		expression_token_t *expr;
		if (*current == 0) {
			break;
		} else if (isdigit(*current)) {
			expr = parse_digit(&current);
		} else if (*current == '(') {
			expr = malloc(sizeof(expression_token_t));
			expr->type = OPEN_PARENTHESIS;
			stack_push(stack, expr);
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
				stack_pop(stack);
				list_add(list->tokens, expr);

				if (stack->length <= 0) {
					expr = 0;
					return 0;
				}

				expr = stack_peek(stack);
			}

			stack_pop(stack);
			free(expr);
			current++;
			continue;
		} else if(strchr(operator_cache, *current)) {
			expr = parse_operator(&current);
		} else {
			expr = parse_symbol(&current);
			list_add(list->symbols, expr->symbol);
		}

		if (expr->type == OPERATOR) {
			operator_t *operator = &operators[expr->operator];
			expression_token_t *expr2 = stack->length ? stack_peek(stack) : 0;
			operator_t *operator2 = expr2 ? &operators[expr2->operator] : 0;
			while (expr2 && expr2->type == OPERATOR && ((!operator->right_assocative && operator2->precedence == operator->precedence) || operator->precedence < operator2->precedence)) {
				stack_pop(stack);
				list_add(list->tokens, expr2);
				if (!stack->length) {
					break;
				}
				expr2 = stack_peek(stack);
				operator2 = &operators[expr2->operator];
			}
			stack_push(stack, expr);
		} else {
			list_add(list->tokens, expr);
		}
	}

	while (stack->length > 0) {
		expression_token_t *item = stack_pop(stack);
		list_add(list->tokens, item);
	}

	stack_free(stack);
	return list;
}
