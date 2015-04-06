#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include "log.h"
#include "stringop.h"
#include "list.h"
#include "enums.h"
#include "errors.h"
#include "assembler.h"
#include "linker.h"
#include "merge.h"
#include "expression.h"
#include "runtime.h"
#include "bin.h"
#include "8xp.h"
#ifdef _WIN32
#include <Windows.h>
#else
#include <strings.h>
#endif

struct output_format {
	char *name;
	format_writer writer;
};

/* Keep alphabetized */
struct output_format output_formats[] = {
	{ "bin", output_bin },
	{ "8xp", output_8xp }
};

struct runtime scas_runtime;

void init_scas_runtime() {
	scas_runtime.arch = "z80";
	scas_runtime.jobs = LINK | ASSEMBLE;
	scas_runtime.output_type = EXECUTABLE;
	scas_runtime.input_files = create_list();
	scas_runtime.output_file = NULL;
	scas_runtime.listing_file = NULL;
	scas_runtime.symbol_file = NULL;
	scas_runtime.include_path = getenv("SCAS_PATH");
	if (!scas_runtime.include_path) {
		scas_runtime.include_path = malloc(3);
		strcpy(scas_runtime.include_path, "./");
	}
	scas_runtime.linker_script = NULL;
	scas_runtime.verbosity = L_SILENT;

	scas_runtime.options.explicit_export = false;
	scas_runtime.options.explicit_import = true;
	scas_runtime.options.auto_relocation = false;
	scas_runtime.options.remove_unused_functions = true;
	scas_runtime.options.output_format = output_bin;
	scas_runtime.options.prog_name_8xp = "SCAS";
	scas_runtime.options.prog_protected_8xp = true;
}

void validate_scas_runtime() {
	if (scas_runtime.input_files->length == 0) {
		scas_abort("No input files given");
	}
	if (scas_runtime.output_file == NULL) {
		/* Auto-assign an output file name */
		const char *ext;
		if ((scas_runtime.jobs & LINK) == LINK) {
			ext = ".bin";
		} else {
			ext = ".o";
		}
		scas_runtime.output_file = malloc(strlen(scas_runtime.input_files->items[0]) + strlen(ext) + 1);
		strcpy(scas_runtime.output_file, scas_runtime.input_files->items[0]);
		int i = strlen(scas_runtime.output_file);
		while (scas_runtime.output_file[--i] != '.' && i != 0);
		if (i == 0) {
			i = strlen(scas_runtime.output_file);
		}
		int j;
		for (j = 0; j < sizeof(ext); j++) {
			scas_runtime.output_file[i + j] = ext[j];
		}
		scas_log(L_DEBUG, "Assigned output file name to %s", scas_runtime.output_file);
	}
}

int format_compare(const void *_a, const void *_b) {
	const struct output_format *a = _a;
	const struct output_format *b = _b;
	return strcasecmp(a->name, b->name);
}

void parse_flag(const char *flag) {
	flag += 2;
	char *name = NULL;
	char *value = NULL;
	if ((value = strchr(flag, '='))) {
		name = malloc(value - flag + 1);
		strncpy(name, flag, value - flag);
		name[value - flag] = '\0';
		value++;
	} else {
		name = malloc(strlen(flag) + 1);
		strcpy(name, flag);
		value = "yes";
	}

	bool yes = !strcasecmp("yes", value);
	if (!strcasecmp("no", value)) {
		yes = false;
	}

	if (strcmp("explicit-export", name) == 0) {
		scas_runtime.options.explicit_export = yes;
	} else if (strcmp("no-explicit-export", name) == 0) {
		scas_runtime.options.explicit_export = !yes;
	} else if (strcmp("explicit-import", name) == 0) {
		scas_runtime.options.explicit_import = yes;
	} else if (strcmp("no-explicit-import", name) == 0) {
		scas_runtime.options.explicit_import = !yes;
	} else if (strcmp("auto-relocation", name) == 0) {
		scas_runtime.options.auto_relocation = yes;
	} else if (strcmp("no-auto-relocation", name) == 0) {
		scas_runtime.options.auto_relocation = !yes;
	} else if (strcmp("remove-unused-funcs", name) == 0) {
		scas_runtime.options.remove_unused_functions = yes;
	} else if (strcmp("no-remove-unused-funcs", name) == 0) {
		scas_runtime.options.remove_unused_functions = !yes;
	} else if (strcmp("format", name) == 0) {
		struct output_format o = { .name=value };
		struct output_format *res =
			bsearch(&o, output_formats,
				sizeof(output_formats) / sizeof(struct output_format),
				sizeof(struct output_format), format_compare);
		if (!res) {
			scas_abort("Unknown output format %s", value);
		}
		scas_runtime.options.output_format = res->writer;
	} else if (strcmp("prog-name", name) == 0) {
		if (strlen(value) > 8) {
			scas_abort("-fprog-name must be 8 characters or fewer.");
		}
		char *v = value;
		while (*v) {
			if (!isupper(*v) || !isascii(*v)) {
				scas_abort("-fprog-name must be all uppercase ASCII.");
			}
			v++;
		}
		scas_runtime.options.prog_name_8xp = value;
	} else if (strcmp("prog-protected", name) == 0) {
		scas_runtime.options.prog_protected_8xp = yes;
	} else if (strcmp("no-prog-protected", name) == 0) {
		scas_runtime.options.prog_protected_8xp = !yes;
	} else {
		scas_abort("Unknown flag %s", name);
	}

	free(name);
}

void parse_arguments(int argc, char **argv) {
	int i;
	for (i = 1; i < argc; ++i) {
		if (argv[i][0] == '-' && argv[i][1] != '\0') {
			if (strcmp("-o", argv[i]) == 0 || strcmp("--output", argv[i]) == 0) {
				scas_runtime.output_file = argv[++i];
			} else if (strcmp("-i", argv[i]) == 0 || strcmp("--input", argv[i]) == 0) {
				list_add(scas_runtime.input_files, argv[++i]);
			} else if (strcmp("-c", argv[i]) == 0 || strcmp("--merge", argv[i]) == 0) {
				scas_runtime.jobs &= ~LINK;
			} else if (argv[i][1] == 'f') {
				parse_flag(argv[i]);
			} else if (argv[i][1] == 'I' || strcmp("--include", argv[i]) == 0) {
				char *path;
				if (argv[i][1] == 'I' && argv[i][2] != 0) {
					// -I/path/goes/here
					path = argv[i] + 2;
				} else {
					// [-I | --include] path/goes/here
					path = argv[++i];
				}
				int l = strlen(scas_runtime.include_path);
				scas_runtime.include_path = realloc(scas_runtime.include_path, l + strlen(path) + 2);
				strcat(scas_runtime.include_path, ":");
				strcat(scas_runtime.include_path, path);
			} else if (argv[i][1] == 'v') {
				int j;
				for (j = 1; argv[i][j] != '\0'; ++j) {
					if (argv[i][j] == 'v') {
						scas_runtime.verbosity++;
					} else {
						scas_abort("Invalid option %s", argv[i]);
					}
				}
			} else {
				scas_abort("Invalid option %s", argv[i]);
			}
		} else {
			if (scas_runtime.output_file != NULL || i != argc - 1 || scas_runtime.input_files->length == 0) {
				scas_log(L_INFO, "Added input file '%s'", argv[i]);
				list_add(scas_runtime.input_files, argv[i]);
			} else if (scas_runtime.output_file == NULL && i == argc - 1) {
				scas_runtime.output_file = argv[i];
			}
		}
	}
}

instruction_set_t *find_inst() {
	const char *sets_dir = INSTRUCTION_SET_PATH;
	const char *ext = ".tab";
	FILE *f = fopen(scas_runtime.arch, "r");
	if (!f) {
		char *path = malloc(strlen(scas_runtime.arch) + strlen(sets_dir) + strlen(ext) + 1);
		strcpy(path, sets_dir);
		strcat(path, scas_runtime.arch);
		strcat(path, ext);
		f = fopen(path, "r");
		free(path);
		if (!f) {
			scas_abort("Unknown architecture: %s", scas_runtime.arch);
		}
	}
	instruction_set_t *set = load_instruction_set(f);
	fclose(f);
	return set;
}

list_t *split_include_path() {
	list_t *list = create_list();
	int i, j;
	for (i = 0, j = 0; scas_runtime.include_path[i]; ++i) {
		if (scas_runtime.include_path[i] == ':' || scas_runtime.include_path[i] == ';') {
			char *s = malloc(i - j + 1);
			strncpy(s, scas_runtime.include_path + j, i - j);
			s[i - j] = '\0';
			j = i + 1;
			list_add(list, s);
		}
	}
	char *s = malloc(i - j + 1);
	strncpy(s, scas_runtime.include_path + j, i - j);
	s[i - j] = '\0';
	list_add(list, s);
	return list;
}

int main(int argc, char **argv) {
	init_scas_runtime();
	parse_arguments(argc, argv);
	init_log(scas_runtime.verbosity);
	validate_scas_runtime();
	instruction_set_t *instruction_set = find_inst();
	scas_log(L_INFO, "Loaded instruction set: %s", instruction_set->arch);
	list_t *include_path = split_include_path();
	list_t *errors = create_list();
	list_t *warnings = create_list();

	list_t *objects = create_list();
	int i;
	for (i = 0; i < scas_runtime.input_files->length; ++i) {
		scas_log(L_INFO, "Assembling input file: '%s'", scas_runtime.input_files->items[i]);
		indent_log();
		FILE *f;
		if (strcasecmp(scas_runtime.input_files->items[i], "-") == 0) {
			f = stdin;
		} else {
			f = fopen(scas_runtime.input_files->items[i], "r");
		}
		if (!f) {
			scas_abort("Unable to open '%s' for assembly.", scas_runtime.input_files->items[i]);
		}
		char magic[7];
		bool is_object = false;
		if (fread(magic, sizeof(char), 7, f) == 7) {
			if (strncmp("SCASOBJ", magic, 7) == 0) {
				is_object = true;
			}
		}
		fseek(f, 0L, SEEK_SET);

		object_t *o;
		if (is_object) {
			scas_log(L_INFO, "Loading object file '%s'", scas_runtime.input_files->items[i]);
			o = freadobj(f, scas_runtime.input_files->items[i]);
		} else {
			assembler_settings_t settings = {
				.include_path = include_path,
				.set = instruction_set,
				.errors = errors,
				.warnings = warnings,
			};
			o = assemble(f, scas_runtime.input_files->items[i], &settings);
			fclose(f);
			scas_log(L_INFO, "Assembler returned %d errors, %d warnings for '%s'",
					errors->length, warnings->length, scas_runtime.input_files->items[i]);
		}
		list_add(objects, o);
		deindent_log();
	}

	scas_log(L_DEBUG, "Opening output file for writing: %s", scas_runtime.output_file);
	FILE *out;
	if (strcasecmp(scas_runtime.output_file, "-") == 0) {
		out = stdout;
	} else {
		out = fopen(scas_runtime.output_file, "w+");
	}
	if (!out) {
		scas_abort("Unable to open '%s' for output.", scas_runtime.output_file);
	}

	if ((scas_runtime.jobs & LINK) == LINK) {
		scas_log(L_INFO, "Passing objects to linker");
		linker_settings_t settings = {
			.automatic_relocation = scas_runtime.options.auto_relocation,
			.merge_only = (scas_runtime.jobs & MERGE) == MERGE,
			.errors = errors,
			.warnings = warnings,
			.write_output = scas_runtime.options.output_format
		};
		if (settings.merge_only) {
			object_t *merged = merge_objects(objects);
			fwriteobj(out, merged);
		} else {
			link_objects(out, objects, &settings);
		}
		scas_log(L_INFO, "Linker returned %d errors, %d warnings", errors->length, warnings->length);
	} else {
		scas_log(L_INFO, "Skipping linking - writing to object file");
		object_t *merged = merge_objects(objects);
		fwriteobj(out, merged);
		fflush(out);
		fclose(out);
	}
	if (errors->length != 0) {
		int i;
		for (i = 0; i < errors->length; ++i) {
			error_t *error = errors->items[i];
			fprintf(stderr, "%s:%d:%d: error #%d: %s\n", error->file_name,
					(int)error->line_number, (int)error->column, error->code,
					get_error_string(error));
			fprintf(stderr, "%s\n", error->line);
			if (error->column != 0) {
				int j;
				for (j = error->column; j > 0; --j) {
					fprintf(stderr, ".");
				}
				fprintf(stderr, "^\n");
			} else {
				fprintf(stderr, "\n");
			}
		}
		remove(scas_runtime.output_file);
	}
	if (warnings->length != 0) {
		int i;
		for (i = 0; i < errors->length; ++i) {
			warning_t *warning = warnings->items[i];
			fprintf(stderr, "%s:%d:%d: warning #%d: %s\n", warning->file_name,
					(int)warning->line_number, (int)warning->column, warning->code,
					get_warning_string(warning));
			fprintf(stderr, "%s\n", warning->line);
			if (warning->column != 0) {
				int j;
				for (j = warning->column; j > 0; --j) {
					fprintf(stderr, ".");
				}
				fprintf(stderr, "^\n");
			}
		}
	}

	int ret = errors->length;
	scas_log(L_DEBUG, "Exiting with status code %d, cleaning up", ret);
	list_free(scas_runtime.input_files);
	free_flat_list(include_path);
	list_free(objects);
	list_free(errors);
	list_free(warnings);
	instruction_set_free(instruction_set);
	return ret;
}
