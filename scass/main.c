#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include "log.h"
#include "enums.h"

struct {
	char *arch;
	jobs_t jobs;
	int explicit_export;
	int explicit_import;
	output_type_t output_type;
	int input_files_len;
	int input_files_capacity;
	char **input_files;
	char *output_file;
	char *listing_file;
	char *symbol_file;
	char *include_path;
	char *linker_script;
	int verbosity;
} runtime;

void init_runtime() {
	runtime.arch = "z80";
	runtime.jobs = LINK | ASSEMBLE;
	runtime.explicit_import = 1;
	runtime.explicit_export = 0;
	runtime.output_type = EXECUTABLE;
	runtime.input_files = malloc(sizeof(char*) * 10);
	runtime.input_files_len = 0;
	runtime.input_files_capacity = 10;
	runtime.output_file = NULL;
	runtime.listing_file = NULL;
	runtime.symbol_file = NULL;
	runtime.include_path = getenv("SCASS_PATH");
	runtime.linker_script = NULL;
	runtime.verbosity = L_WARNING;
}

void validate_runtime() {
	if (runtime.input_files_len == 0) {
		scass_abort("No input files given");
	}
	if (runtime.output_file == NULL) {
		/* Auto-assign an output file name */
		const char *bin = ".bin";
		runtime.output_file = malloc(strlen(runtime.input_files[0]) + sizeof(bin));
		memcpy(runtime.output_file, runtime.input_files[0], strlen(runtime.input_files[0]));
		int i = strlen(runtime.output_file);
		while (runtime.output_file[--i] != '.' && i != 0);
		if (i == 0) {
			i = strlen(runtime.output_file);
		}
		int j;
		for (j = 0; j < sizeof(bin); j++) {
			runtime.output_file[i + j] = bin[j];
		}
	}
}

void runtime_add_input_file(char *file) {
	if (runtime.input_files_capacity == runtime.input_files_len) {
		runtime.input_files_capacity += 10;
		runtime.input_files = realloc(runtime.input_files, sizeof(char*) * runtime.input_files_capacity);
		if (runtime.input_files == NULL) {
			scass_abort("Couldn't resize input file buffer");
		}
	}
	runtime.input_files[runtime.input_files_len++] = file;
}

void parse_arguments(int argc, char **argv) {
	int i;
	for (i = 1; i < argc; ++i) {
		if (argv[i][0] == '-') {
			if (strcasecmp("-o", argv[i]) == 0 || strcasecmp("--output", argv[i]) == 0) {
				runtime.output_file = argv[++i];
			} else if (strcasecmp("-i", argv[i]) == 0 || strcasecmp("--input", argv[i]) == 0) {
				runtime_add_input_file(argv[++i]);
			} else if (strcasecmp("-l", argv[i]) == 0 || strcasecmp("--link", argv[i]) == 0) {
				runtime.jobs = LINK;
			} else if (strcasecmp("-O", argv[i]) == 0 || strcasecmp("--object", argv[i]) == 0) {
				runtime.jobs = ASSEMBLE;
			} else if (strcasecmp("-e", argv[i]) == 0 || strcasecmp("--export-explicit", argv[i]) == 0) {
				runtime.explicit_export = 1;
			} else if (strcasecmp("-n", argv[i]) == 0 || strcasecmp("--no-implicit-symbols", argv[i]) == 0) {
				runtime.explicit_import = 0;
			} else if (argv[i][1] == 'v') {
				int j;
				for (j = 1; argv[i][j] != '\0'; ++j) {
					if (argv[i][j] == 'v') {
						runtime.verbosity++;
					} else {
						scass_abort("Invalid option %s", argv[i]);
					}
				}
			} else {
				scass_abort("Invalid option %s", argv[i]);
			}
		} else {
			if (runtime.output_file != NULL || i != argc - 1 || runtime.input_files_len == 0) {
				runtime_add_input_file(argv[i]);
			} else if (runtime.output_file == NULL && i == argc - 1) {
				runtime.output_file = argv[i];
			}
		}
	}
}

int main(int argc, char **argv) {
	init_runtime();
	parse_arguments(argc, argv);
	init_log(runtime.verbosity);
	validate_runtime();
	int i;
	for (i = 0; i < runtime.input_files_len; ++i) {
		printf("Input: %s\n", runtime.input_files[i]);
	}
	printf("Output: %s\n", runtime.output_file);
	return 0;
}
