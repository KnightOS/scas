#ifndef LOG_H
#define LOG_H

typedef enum {
    L_ERROR = 0,
    L_WARNING,
    L_INFO,
    L_DEBUG,
} log_importance_t;

void init_log(int verbosity);
void scas_log(int verbosity, char* format, ...);
void scas_abort(char* format, ...);
void indent_log();
void deindent_log();

#endif
