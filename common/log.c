#include "log.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

int v = 0;
int indent = 0;

const char *verbosity_colors[] = {
	"\x1B[1;31m", // L_ERROR
	"\x1B[1;33m", // L_WARNING
	"\x1B[1;34m", // L_INFO
	"\x1B[1;30m", // L_DEBUG
};

void init_log(int verbosity) {
	v = verbosity;
}

void indent_log() {
	++indent;
}

void deindent_log() {
	if (indent > 0) {
		--indent;
	}
}

void scas_abort(char *format, ...) {
	fprintf(stderr, "ERROR: ");
	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
	fprintf(stderr, "\n");
	exit(1);
}

void scas_log(int verbosity, char* format, ...) {
	if (verbosity <= v) {
		int c = verbosity;
		if (c > sizeof(verbosity_colors) / sizeof(char *)) {
			c = sizeof(verbosity_colors) / sizeof(char *) - 1;
		}
		fprintf(stderr, verbosity_colors[c]);
		int i;
		for (i = 0; i < indent; ++i) {
			fprintf(stderr, "..");
		}
		va_list args;
		va_start(args, format);
		vfprintf(stderr, format, args);
		va_end(args);
		fprintf(stderr, "\x1B[0m\n");
	}
}
