typedef enum {
    L_SILENT = 0,
    L_ERROR = 1,
    L_INFO = 2,
    L_DEBUG = 3,
} scas_log_importance_t;

void scas_log(scas_log_importance_t verbosity, char* format, ...);

extern bool scas_log_colorize;
extern scas_log_importance_t scas_log_verbosity;
extern unsigned scas_log_indent;
