#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#ifdef _WIN32
#include <Windows.h>
#define strcmp lstrcmpA
#else
#include <strings.h>
#endif
#include "enums.h"
#include "log.h"

struct {
	char *arch;
	jobs_t jobs;
	int explicit_export;
	int explicit_import;
	output_type_t output_type;
	int input_files_len;
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
	runtime.explicit_import = runtime.explicit_export = 0;
	runtime.output_type = EXECUTABLE;
	runtime.input_files = NULL;
	runtime.output_file = NULL;
	runtime.listing_file = NULL;
	runtime.symbol_file = NULL;
	runtime.include_path = getenv("SASS_PATH");
	runtime.linker_script = NULL;
	runtime.verbosity = 0;
}

void sass_abort(char *format, ...) {
	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
	fprintf(stderr, "\n");
	exit(1);
}

#define match(short, long) strcmp(short, argv[i]) == 0 || strcmp(long, argv[i]) == 0

void parse_arguments(int argc, char** argv) {
	int i;
	for (i = 0; i < argc; ++i) {
		if (argv[i][0] == '-' && argv[i][1] != '\0') {
			if (match("-a", "--architecture")) {
				runtime.arch = argv[++i];
			} else if (match("-D", "--define")) {
				// TODO
			} else if (match("-e", "--export-explicit")) {
				runtime.explicit_export = 1;
			} else if (match("-i", "--input")) {
				// TODO
			} else if (match("-I", "--include")) {
				// TODO
			} else if (match("-o", "--output")) {
				runtime.output_file = argv[++i];
			} else if (match("-O", "--object")) {
				runtime.jobs = ASSEMBLE;
			} else if (match("-l", "--link")) {
				runtime.jobs = LINK;
			} else if (match("-L", "--listing")) {
				runtime.listing_file = argv[++i];
			} else if (match("-n", "--no-implicit-symbols")) {
				runtime.explicit_import = 1;
			} else if (match("-s", "--script")) {
				runtime.linker_script = argv[++i];
			} else if (match("-S", "--symbols")) {
				runtime.symbol_file = argv[++i];
			} else if (argv[i][1] == 'v') {
				int j;
				for (j = 1; j < strlen(argv[i]); ++j) {
					if (argv[i][j] == 'v') {
						++runtime.verbosity;
					} else {
						sass_abort("Unrecognized option %s", argv[i]);
					}
				}
			} else {
				sass_abort("Unrecognized option %s", argv[i]);
			}
		} else {
			// TODO
		}
	}
}

void validate_runtime() {
	// TODO: Check for illegal states
}

int main(int argc, char** argv) {
	init_runtime();
	parse_arguments(argc, argv);
	init_log(runtime.verbosity);
	validate_runtime();
	return 0;
}
