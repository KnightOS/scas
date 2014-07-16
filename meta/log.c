#include "log.h"
#include <stdarg.h>
#include <stdio.h>

int v = 0;

void init_log(int verbosity) {
	v = verbosity;
}

void sass_log(int verbosity, char* format, ...) {
	if (verbosity <= v) {
		va_list args;
		va_start(args, format);
		vfprintf(stderr, format, args);
		va_end(args);
		fprintf(stderr, "\n");
	}
}
