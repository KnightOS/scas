#ifndef ERRORS_H
#define ERRORS_H
#include "list.h"
#include <stdint.h>
#include <stddef.h>

enum {
    ERROR_INVALID_INSTRUCTION = 1
};

typedef struct {
    int code;
    size_t line_number;
    char *file_name;
    char *line;
    size_t column;
} error_t;

typedef struct {
    int code;
    size_t line_number;
    char *file_name;
    char *line;
    size_t column;
} warning_t;

const char *get_error_string(error_t *error);
const char *get_warning_string(warning_t *warning);
void add_error(list_t *errors, int code, size_t line_number, const char *line, int column, const char *file_name);

#endif
