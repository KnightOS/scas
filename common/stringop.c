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
