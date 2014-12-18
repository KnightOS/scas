#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "log.h"
#include "list.h"
#include "objects.h"
#include "expression.h"

struct {
	list_t *input_files;
	char *area;
	int dump_code;
	int dump_private_symbols;
	int dump_public_symbols;
	int dump_references;
	int dump_machine_code;
} runtime;

void init_runtime() {
	runtime.input_files = create_list();
	runtime.area = NULL;
	runtime.dump_code = 0;
	runtime.dump_private_symbols = 0;
	runtime.dump_public_symbols = 0;
	runtime.dump_references = 0;
	runtime.dump_machine_code = 0;
}

void parse_arguments(int argc, char **argv) {
	int i;
	for (i = 1; i < argc; ++i) {
		if (argv[i][0] == '-' && argv[i][1] != '\0') {
			if (strcmp("-i", argv[i]) == 0 || strcmp("--input", argv[i]) == 0) {
				list_add(runtime.input_files, argv[++i]);
			} else if (strcmp("-a", argv[i]) == 0 || strcmp("--area", argv[i]) == 0) {
				runtime.area = argv[++i];
			} else if (strcmp("-c", argv[i]) == 0 || strcmp("--code", argv[i]) == 0) {
				runtime.dump_code = 1;
			} else if (strcmp("-p", argv[i]) == 0 || strcmp("--private-symbols", argv[i]) == 0) {
				runtime.dump_private_symbols = 1;
			} else if (strcmp("-s", argv[i]) == 0 || strcmp("--symbols", argv[i]) == 0) {
				runtime.dump_public_symbols = 1;
			} else if (strcmp("-r", argv[i]) == 0 || strcmp("--references", argv[i]) == 0) {
				runtime.dump_references = 1;
			} else if (strcmp("-x", argv[i]) == 0 || strcmp("--machine-code", argv[i]) == 0) {
				runtime.dump_machine_code = 1;
			} else {
				scas_abort("Invalid option %s", argv[i]);
			}
		} else {
			list_add(runtime.input_files, argv[i]);
		}
	}
}

int main(int argc, char **argv) {
	init_runtime();
	parse_arguments(argc, argv);
	return 0;
}
