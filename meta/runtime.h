#ifndef RUNTIME_H
#define RUNTIME_H

#include "enums.h"

struct runtime {
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
};

extern struct runtime runtime;

void init_runtime();
void validate_runtime();
#endif
