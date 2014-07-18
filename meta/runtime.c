#include "runtime.h"
#include "log.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct runtime runtime;

void init_runtime() {
	runtime.arch = "z80";
	runtime.jobs = LINK | ASSEMBLE;
	runtime.explicit_import = runtime.explicit_export = 0;
	runtime.output_type = EXECUTABLE;
	runtime.input_files = malloc(sizeof(char*) * 10);
	runtime.input_files_len = 0;
	runtime.input_files_capacity = 10;
	runtime.output_file = NULL;
	runtime.listing_file = NULL;
	runtime.symbol_file = NULL;
	runtime.include_path = getenv("SASS_PATH");
	runtime.linker_script = NULL;
	runtime.verbosity = 0;
}

void validate_runtime() {
	if (runtime.input_files_len == 0) {
		sass_abort("No input files given!");
	}
	// TODO: Check for illegal states
}

void runtime_add_input_file(char *file) {
	if (runtime.input_files_capacity == runtime.input_files_len) {
		runtime.input_files_capacity += 10;
		runtime.input_files = realloc(runtime.input_files, sizeof(char*) * runtime.input_files_capacity);
		if (runtime.input_files == NULL) {
			sass_abort("Couldn't resize input file buffer!");
		}
	}
	runtime.input_files[runtime.input_files_len++] = file;
}
