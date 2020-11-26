#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "log.h"

#ifdef EMSCRIPTEN
#include <emscripten.h>
#endif

void scas_log_default(const char *msg) {
	fprintf(stderr, "%s", msg);
}

scas_log_importance_t scas_log_verbosity = 0;
unsigned scas_log_indent = 0;

#ifdef __ELF__
bool scas_log_colorize = true;
#else
bool scas_log_colorize = false;
#endif

void (*scas_log_function)(const char *) = scas_log_default;

const char *verbosity_colors[] = {
	"", // L_SILENT
	"\x1B[1;31m", // L_ERROR
	"\x1B[1;34m", // L_INFO
	"\x1B[1;30m", // L_DEBUG
};

void scas_log(scas_log_importance_t verbosity, char* format, ...) {
	if (scas_log_function) {
		if (verbosity <= scas_log_verbosity && verbosity >= 0) {
			size_t c = verbosity;
			if (c > sizeof(verbosity_colors) / sizeof(char *)) {
				c = sizeof(verbosity_colors) / sizeof(char *) - 1;
			}
			if (scas_log_colorize) {
				scas_log_function(verbosity_colors[c]);
			}
			if (verbosity == L_DEBUG || verbosity == L_INFO) {
				for (unsigned i = 0; i < scas_log_indent; ++i) {
					scas_log_function("  ");
				}
			}
			va_list args;
			va_start(args, format);
			int length = vsnprintf(NULL, 0, format, args);
			va_end(args);
			if (length > 0) {
				va_start(args, format);
				length += 1;
				char *buf = malloc(length);
				vsnprintf(buf, length, format, args);
				va_end(args);
				scas_log_function(buf);
				free(buf);
				scas_log_function(scas_log_colorize ? "\n\x1B[0m" : "\n");
			}
		}
	}
}
