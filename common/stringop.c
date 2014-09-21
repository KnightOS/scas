#include "stringop.h"
#include <stdlib.h>
#include "string.h"
#include "strings.h"

char *strip_whitespace(char *_str) {
	if (*_str == '\0')
		return _str;
	char *strold = _str;
	while (*_str == ' ' || *_str == '\t') {
		_str++;
	}
	char *str = malloc(strlen(_str) + 1);
	strcpy(str, _str);
	free(strold);
	int i;
	for (i = 0; str[i] != '\0'; ++i);
	do {
		i--;
	} while (str[i] == ' ' || str[i] == '\t');
	str[i + 1] = '\0';
	return str;
}

char *strip_comments(char *str) {
	int in_string = 0, in_character = 0;
	int i = 0;
	while (str[i] != '\0') {
		if (str[i] == '"' && !in_character) {
			in_string = !in_string;
		} else if (str[i] == '\'' && !in_string) {
			in_character = !in_character;
		} else if (!in_character && !in_string) {
			if (str[i] == ';') {
				str[i] = '\0';
				break;
			}
		}
		++i;
	}
	return str;
}
