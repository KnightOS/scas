#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include "log.h"
#include "list.h"
#include "enums.h"
#include "assembler.h"

struct {
	char *arch;
	jobs_t jobs;
	int explicit_export;
	int explicit_import;
	output_type_t output_type;
	list_t *input_files;
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
	runtime.input_files = create_list();
	runtime.output_file = NULL;
	runtime.listing_file = NULL;
	runtime.symbol_file = NULL;
	runtime.include_path = getenv("SCAS_PATH");
	runtime.linker_script = NULL;
	runtime.verbosity = L_WARNING;
}

void validate_runtime() {
	if (runtime.input_files->length == 0) {
		scas_abort("No input files given");
	}
	if (runtime.output_file == NULL) {
		/* Auto-assign an output file name */
		const char *bin = ".bin";
		runtime.output_file = malloc(strlen(runtime.input_files->items[0]) + sizeof(bin));
		memcpy(runtime.output_file, runtime.input_files->items[0], strlen(runtime.input_files->items[0]));
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

void parse_arguments(int argc, char **argv) {
	int i;
	for (i = 1; i < argc; ++i) {
		if (argv[i][0] == '-' && argv[i][1] != '\0') {
			if (strcasecmp("-o", argv[i]) == 0 || strcasecmp("--output", argv[i]) == 0) {
				runtime.output_file = argv[++i];
			} else if (strcasecmp("-i", argv[i]) == 0 || strcasecmp("--input", argv[i]) == 0) {
				list_add(runtime.input_files, argv[++i]);
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
						scas_abort("Invalid option %s", argv[i]);
					}
				}
			} else {
				scas_abort("Invalid option %s", argv[i]);
			}
		} else {
			if (runtime.output_file != NULL || i != argc - 1 || runtime.input_files->length == 0) {
				list_add(runtime.input_files, argv[i]);
			} else if (runtime.output_file == NULL && i == argc - 1) {
				runtime.output_file = argv[i];
			}
		}
	}
}

instruction_set_t *find_inst() {
	const char *sets_dir = INSTRUCTION_SET_PATH;
	const char *ext = ".tab";
	FILE *f = fopen(runtime.arch, "r");
	if (!f) {
		char *path = malloc(strlen(runtime.arch) + strlen(sets_dir) + strlen(ext) + 1);
		strcpy(path, sets_dir);
		strcat(path, runtime.arch);
		strcat(path, ext);
		f = fopen(path, "r");
		free(path);
		if (!f) {
			scas_abort("Unknown architecture: %s", runtime.arch);
		}
	}
	instruction_set_t *set = load_instruction_set(f);
	fclose(f);
	return set;
}

int main(int argc, char **argv) {
	init_runtime();
	parse_arguments(argc, argv);
	init_log(runtime.verbosity);
	validate_runtime();
	instruction_set_t *instruction_set = find_inst();
	list_t *objects = create_list();
	if ((runtime.jobs & ASSEMBLE) == ASSEMBLE) {
		int i;
		for (i = 0; i < runtime.input_files->length; ++i) {
			FILE *f;
			if (strcasecmp(runtime.input_files->items[i], "-") == 0) {
				f = stdin;
			} else {
				f = fopen(runtime.input_files->items[i], "r");
			}
			if (!f) {
				scas_abort("Unable to open '%s' for assembly.", runtime.input_files->items[i]);
			}
			object_t *o = assemble(f, instruction_set);
			fclose(f);
			list_add(objects, o);
		}
	}
	list_free(runtime.input_files);
	list_free(objects);
	return 0;
}
