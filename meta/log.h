#ifndef LOG_H
#define LOG_H

typedef enum {
    ALWAYS,
    HIGH,
    MED,
    LOW
} log_importance_t;

void init_log(int verbosity);
void sass_log(int verbosity, char* format, ...);
void sass_abort(char* format, ...);

#endif
