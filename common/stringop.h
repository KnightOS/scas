#ifndef STRINGOP_H
#define STRINGOP_H
#include "list.h"

char *strip_whitespace(char *str);
char *strip_comments(char *str);
list_t *split_string(const char *str, const char *delims);
void free_string_list(list_t *list);

#endif
