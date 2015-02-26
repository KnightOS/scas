#ifndef GLOBALS_H
#define GLOBALS_H
#include <stdbool.h>
#include "list.h"
#include "enums.h"

struct runtime_options {
	bool explicit_export;			/* -fexplicit-export */
	bool explicit_import;			/* -fexplicit-import */
	bool auto_relocation;			/* -fauto-relocation */
	bool remove_unused_functions;	/* -fremove-unused-funcs */
};

struct runtime {
	char *arch;
	jobs_t jobs;
	output_type_t output_type;
	list_t *input_files;
	char *output_file;
	char *listing_file;
	char *symbol_file;
	char *include_path;
	char *linker_script;
	int verbosity;
	struct runtime_options options;
};

extern struct runtime scas_runtime;

#endif
