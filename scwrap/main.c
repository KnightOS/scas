#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include "log.h"
#include "list.h"
#include "stack.h"
#include "expression.h"
#include "objects.h"

struct {
	char *input_file;
	char *output_file;
	char *prefix;
} runtime;

void init_runtime() {
	runtime.input_file = NULL;
	runtime.output_file = NULL;
	runtime.prefix = NULL;
}

bool parse_arguments(int argc, char **argv) {
	if (argc != 4) {
		scas_log(L_ERROR, "Invalid usage. See `man scwrap` for usage info.");
		return false;
	}
	runtime.input_file = argv[1];
	runtime.output_file = argv[2];
	runtime.prefix = argv[3];
	return true;
}

int main(int argc, char **argv) {
	init_runtime();
	if (!parse_arguments(argc, argv)) {
		return 1;
	}
	scas_log_init(L_INFO);

	object_t *o = create_object();
	area_t *a = create_area("DATA");
	list_add(o->areas, a);

	FILE *in;
	if (strcasecmp(runtime.input_file, "-") == 0) {
		in = stdin;
	} else {
		in = fopen(runtime.input_file, "r");
	}
	if (!in) {
		scas_log(L_ERROR, "Unable to open input file '%s'.", runtime.input_file);
		return 1;
	}

	const char *_end = "_end";
	const char *_start = "_start";

	fseek(in, 0L, SEEK_END);
	uint64_t len = (uint64_t)ftell(in);
	fseek(in, 0L, SEEK_SET);
	symbol_t start, end;

	start.type = SYMBOL_LABEL;
	start.value = 0;
	start.defined_address = 0;
	start.exported = 1;
	start.name = malloc(1 + strlen(_start) + strlen(runtime.prefix) + 1);
	strcpy(start.name, "_");
	strcat(start.name, runtime.prefix);
	strcat(start.name, _start);
	list_add(a->symbols, &start);

	end.type = SYMBOL_LABEL;
	end.value = len;
	end.defined_address = 0;
	end.exported = 1;
	end.name = malloc(1 + strlen(_end) + strlen(runtime.prefix) + 1);
	strcpy(end.name, "_");
	strcat(end.name, runtime.prefix);
	strcat(end.name, _end);
	list_add(a->symbols, &end);

	while (!feof(in)) {
		unsigned char buf[512];
		int len = fread(buf, sizeof(unsigned char), 512, in);
		append_to_area(a, buf, len);
	}

	FILE *out;
	if (strcasecmp(runtime.output_file, "-") == 0) {
		out = stdout;
	} else {
		out = fopen(runtime.output_file, "w+");
	}
	if (!out) {
		scas_log(L_ERROR, "Unable to open '%s' for output.", runtime.output_file);
		return 1;
	}
	fwriteobj(out, o);
	return 0;
}
