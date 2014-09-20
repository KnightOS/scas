#ifndef LOG_H
#define LOG_H

typedef enum {
    L_DEBUG,
    L_INFO,
    L_WARNING,
    L_ERROR
} log_importance_t;

void init_log(int verbosity);
void scas_log(int verbosity, char* format, ...);
void scas_abort(char* format, ...);

#endif
