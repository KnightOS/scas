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
#include "runtime.h"
#include "log.h"

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
				if (runtime.input_files_capacity == runtime.input_files_len) {
					runtime.input_files_capacity += 10;
					runtime.input_files = realloc(runtime.input_files, sizeof(char*) * runtime.input_files_capacity);
					if (runtime.input_files == NULL) {
						sass_abort("Couldn't resize input file buffer!");
					}
				}
				runtime.input_files[runtime.input_files_len++] = argv[++i];
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

int main(int argc, char** argv) {
	init_runtime();
	parse_arguments(argc, argv);
	init_log(runtime.verbosity);
	validate_runtime();
	return 0;
}
