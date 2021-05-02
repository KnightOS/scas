#define _POSIX_C_SOURCE 200809L
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#define _BSD_EXTENSION
#include <ctype.h>

#include "z80.h"
#include "amd64.h"

#include "list.h"
#include "stack.h"
#include "log.h"
#include "stringop.h"
#include "enums.h"
#include "errors.h"
#include "expression.h"
#include "objects.h"
#include "instructions.h"
#include "assembler.h"
#include "8xp.h"
#include "bin.h"
#include "linker.h"
#include "merge.h"
#include "runtime.h"

static char *out_name = NULL;

void init_scas_runtime() {
	scas_runtime.arch = "z80";
	scas_runtime.link = 1;
	scas_runtime.macros = create_list();
	scas_runtime.output_type = EXECUTABLE;
	scas_runtime.input_names = create_list();
	scas_runtime.input_files = create_list();
	scas_runtime.output_file = NULL;
	scas_runtime.output_extension = "bin";
	scas_runtime.listing_file = NULL;
	scas_runtime.symbol_file = NULL;
	scas_runtime.include_path = getenv("SCAS_PATH");
	if (!scas_runtime.include_path) {
		scas_runtime.include_path = malloc(3);
		strcpy(scas_runtime.include_path, "./");
	}
	scas_runtime.linker_script = NULL;

	scas_runtime.options.explicit_export = false;
	// TODO: reenable
	scas_runtime.options.explicit_import = false;
	scas_runtime.options.auto_relocation = false;
	scas_runtime.options.remove_unused_functions = true;
	scas_runtime.options.output_format = output_bin;
	scas_runtime.options.prog_name_8xp = "SCAS";
	scas_runtime.options.prog_protected_8xp = true;
	scas_runtime.options.origin = 0;
}

static void runtime_open_input(char *name) {
	FILE *f = NULL;
	if (strcasecmp(name, "-") == 0) {
		f = stdin;
	} else {
		f = fopen(name, "r");
	}
	if (!f) {
		scas_log(L_ERROR, "Unable to open '%s' for assembly", name);
		exit(1);
	}
	list_add(scas_runtime.input_names, name);
	list_add(scas_runtime.input_files, f);
}

static void runtime_open(char *name) {
	out_name = strdup(name);
	scas_log(L_DEBUG, "Opening output file for writing: %s", name);
	if (strcasecmp(name, "-") == 0) {
		scas_runtime.output_file = stdout;
	} else {
		scas_runtime.output_file = fopen(name, "w+");
		if (!scas_runtime.output_file) {
			scas_log(L_ERROR, "Unable to open '%s' for output", name);
			exit(1);
		}
	}
}

void validate_scas_runtime() {
	if (scas_runtime.input_files->length == 0) {
		scas_log(L_ERROR, "No input files given");
		exit(1);
	}
	if (scas_runtime.output_file == NULL) {
		/* Auto-assign an output file name */
		const char *ext;
		if (scas_runtime.link) {
			ext = scas_runtime.output_extension;
		} else {
			ext = "o";
		}
		char *out_name = malloc(strlen(scas_runtime.input_names->items[0]) + strlen(ext) + 2);
		strcpy(out_name, scas_runtime.input_names->items[0]);
		int i = strlen(out_name);
		while (out_name[--i] != '.' && i != 0);
		if (i == 0) {
			i = strlen(out_name);
		} else {
			out_name[i] = '\0';
		}
		strcat(out_name, ".");
		strcat(out_name, ext);
		scas_log(L_DEBUG, "Assigned output file name to %s", out_name);
		runtime_open(out_name);
	}
}

/* validate_scas_optarg: ensure that an option that takes an argument was indeed provided an
 *                       argument.
 *  args:
 *    optind: index of the option in _argv_.
 */
void validate_scas_optarg(int optind, int argc, char *argv[]) {
	if (optind == argc - 1 || argv[optind+1][0] == '-') {
		scas_log(L_ERROR, "Option requires argument: %s", argv[optind]);
		exit(1);
	}
}

bool parse_flag(const char *flag) {
	flag += 2;
	char *name, *value;
	value = strchr(flag, '=');
	if (value != NULL) {
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
		if (strcmp(value, "bin") == 0) {
			scas_runtime.options.output_format = output_bin;
		} else if (strcmp(value, "8xp") == 0){
			scas_runtime.options.output_format = output_8xp;
		} else {
			scas_log(L_ERROR, "Unknown output format %s", value);
			return false;
		}
		scas_runtime.output_extension = value;
	} else if (strcmp("8xp-name", name) == 0) {
		if (strlen(value) > 8) {
			scas_log(L_ERROR, "-f8xp-name must be 8 characters or fewer.");
			return false;
		}
		char *v = value;
		while (*v) {
			if (!isupper(*v) || !isascii(*v)) {
				scas_log(L_ERROR, "-f8xp-name must be all uppercase ASCII.");
				return false;
			}
			v++;
		}
		scas_runtime.options.prog_name_8xp = value;
	} else if (strcmp("8xp-protected", name) == 0) {
		scas_runtime.options.prog_protected_8xp = yes;
	} else if (strcmp("no-8xp-protected", name) == 0) {
		scas_runtime.options.prog_protected_8xp = !yes;
	} else if (strcmp("8xp-archived", name) == 0) {
		scas_runtime.options.prog_archived_8xp = yes;
	} else if (strcmp("no-8xp-archived", name) == 0) {
		scas_runtime.options.prog_archived_8xp = !yes;
	} else if (strcmp("origin", name) == 0) {
		tokenized_expression_t *e = parse_expression(value);
		if (!e) {
			scas_log(L_ERROR, "Unable to parse -forigin=%s", value);
			return false;
		}
		list_t *s = create_list();
		int _;
		char *__;
		uint64_t res = evaluate_expression(e, s, &_, &__);
		if (_) {
			scas_log(L_ERROR, "Unable to evaluate -forigin=%s", value);
			return false;
		}
		scas_runtime.options.origin = res;
	} else {
		scas_log(L_ERROR, "Unknown flag %s", name);
		return false;
	}

	free(name);
	return true;
}


void parse_arguments(int argc, char **argv) {
	int i;
	for (i = 1; i < argc; ++i) {
		if (argv[i][0] == '-' && argv[i][1] != '\0') {
			if (strcmp("-o", argv[i]) == 0 || strcmp("--output", argv[i]) == 0) {
				runtime_open(argv[++i]);
			} else if (strcmp("-S", argv[i]) == 0 || strcmp("--symbols", argv[i]) == 0) {
				validate_scas_optarg(i, argc, argv);
				scas_runtime.symbol_file = argv[++i];
			} else if (strcmp("-L", argv[i]) == 0 || strcmp("--listing", argv[i]) == 0) {
				validate_scas_optarg(i, argc, argv);
				scas_runtime.listing_file = argv[++i];
			} else if (strcmp("-i", argv[i]) == 0 || strcmp("--input", argv[i]) == 0) {
				validate_scas_optarg(i, argc, argv);
				runtime_open_input(argv[++i]);
			} else if (strcmp("-c", argv[i]) == 0 || strcmp("--merge", argv[i]) == 0) {
				if(!scas_runtime.link){
					scas_log(L_ERROR, "Error: -c passed multiple times");
					exit(1);
				}
				scas_runtime.link = 0;
			} else if (strcmp("-a", argv[i]) == 0 || strcmp("--architecture", argv[i]) == 0) {
				validate_scas_optarg(i, argc, argv);
				scas_runtime.arch = argv[++i];
			} else if (argv[i][1] == 'f') {
				parse_flag(argv[i]);
			} else if (argv[i][1] == 'I' || strcmp("--include", argv[i]) == 0) {
				char *path;
				if (argv[i][1] == 'I' && argv[i][2] != 0) {
					// -I/path/goes/here
					path = argv[i] + 2;
				} else {
					// [-I | --include] path/goes/here
					validate_scas_optarg(i, argc, argv);
					path = argv[++i];
				}
				int l = strlen(scas_runtime.include_path);
				scas_runtime.include_path = realloc(scas_runtime.include_path, l + strlen(path) + 2);
				strcat(scas_runtime.include_path, ":");
				strcat(scas_runtime.include_path, path);
			} else if (strcmp(argv[i], "--color") == 0){
				scas_log_colorize = true;
			} else if (argv[i][1] == 'v') {
				int j;
				for (j = 1; argv[i][j] != '\0'; ++j) {
					if (argv[i][j] == 'v') {
						scas_log_verbosity++;
					} else {
						scas_log(L_ERROR, "Invalid option %s", argv[i]);
						exit(1);
					}
				}
			} else if (argv[i][1] == 'D' || strcmp("--define", argv[i]) == 0) {
				char *name = NULL, *value = NULL;
				if (argv[i][1] == 'D' && argv[i][2]) {
					name = argv[i] + 2;
				} else {
					validate_scas_optarg(i, argc, argv);
					name = argv[++i];
				}
				value = strchr(name, '=');
				if (value) {
					*value = '\0';
					++value;
				} else {
					value = "1";
				}
				macro_t *macro = malloc(sizeof(macro_t));
				macro->parameters = create_list();
				macro->macro_lines = create_list();
				list_add(macro->macro_lines, strdup(value));
				macro->name = strdup(name);
				list_add(scas_runtime.macros, macro);
			} else if (argv[i][1] == 'h' || strcmp("--help", argv[i]) == 0) {
				printf("Usage: scas [options] FILE(s)\n");
				printf("-c\tassemble only, but do not link\n");
				printf("-I\tadd directories to the include path\n");
				printf("More options by reading the manual (man scas)\n");
				exit(0);
			} else {
				scas_log(L_ERROR, "Invalid option %s", argv[i]);
				exit(1);
			}
		} else {
			if (scas_runtime.output_file || i != argc - 1 || scas_runtime.input_files->length == 0) {
				scas_log(L_INFO, "Added input file '%s'", argv[i]);
				runtime_open_input(argv[i]);
			} else if (scas_runtime.output_file == NULL && i == argc - 1) {
				runtime_open(argv[i]);
			}
		}
	}
}


instruction_set_t *find_inst() {
	const char *sets_dir = "/usr/share/scas/tables/";
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
			// Fall back to internal copy if recognized
			if (strcmp(scas_runtime.arch, "z80") == 0) {
				return load_instruction_set_s(z80_tab);
			} else if (strcmp(scas_runtime.arch, "amd64") == 0) {
				return load_instruction_set_s(amd64_tab);
			} else {
				scas_log(L_ERROR, "Unknown architecture: %s", scas_runtime.arch);
				return NULL;
			}
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

static object_t *
assemble_input(char *name, FILE *f, assembler_settings_t settings)
{
	object_t *o;
	scas_log(L_INFO, "Assembling input file: '%s'", name);
	scas_log_indent += 1;
	char magic[7];
	if(fread(magic, sizeof(char), 7, f) == 7){
		if(strncmp("SCASOBJ", magic, 7) == 0){
			scas_log(L_INFO, "Loading object file '%s'", name);
			rewind(f);
			o = freadobj(f, name);
			scas_log_indent -= 1;
			return o;
		}
	}
	rewind(f);
	o = assemble(f, name, &settings);
	fclose(f);
	scas_log(L_INFO, "Assembler returned %d errors, %d warnings for '%s'",
			settings.errors->length, settings.warnings->length, name);
	scas_log_indent -= 1;
	return o;
}

static void
link(list_t *objects, list_t *errors, list_t *warnings)
{
	scas_log(L_INFO, "Passing objects to linker");
	linker_settings_t settings = {
		.automatic_relocation = scas_runtime.options.auto_relocation,
		.errors = errors,
		.warnings = warnings,
		.write_output = scas_runtime.options.output_format
	};
	link_objects(scas_runtime.output_file, objects, &settings);
	scas_log(L_INFO, "Linker returned %d errors, %d warnings", errors->length, warnings->length);
}

static int
merge(list_t *objects)
{
	int ret;
	object_t *merged;
	scas_log(L_INFO, "Skipping linking - writing to object file");
	merged = merge_objects(objects);
	ret = merged != NULL;
	if(ret){
		fwriteobj(scas_runtime.output_file, merged);
		object_free(merged);
		fflush(scas_runtime.output_file);
	}
	fclose(scas_runtime.output_file);
	return ret;
}

static void
handle_errors(list_t *errors, list_t *warnings)
{
	for (unsigned int i = 0; i < errors->length; ++i) {
		error_t *error = errors->items[i];
		fprintf(stderr, "%s:%lu:%lu: error #%d: %s\n", error->file_name,
				error->line_number, error->column, error->code,
				error->message);
		if(error->line != NULL){
			fputs(error->line, stderr);
			if(error->column != 0){
				for(unsigned int j = error->column; j > 0; j -= 1)
					fputc('.', stderr);
				fputs("^", stderr);
			} else
				fputc('\n', stderr);
			free(error->line);
		}
		free(error->file_name);
		free(error->message);
		free(error);
	}
	// when handle_errors is called, the output fd shall already have been closed
	if(errors->length > 0 && out_name != NULL){
		remove(out_name);
		out_name = NULL;
	}
	for (unsigned int i = 0; i < warnings->length; ++i) {
		warning_t *warning = warnings->items[i];
		fprintf(stderr, "%s:%lu:%lu: warning #%d: %s\n", warning->file_name,
				warning->line_number, warning->column, warning->code,
				get_warning_string(warning));
		if(warning->line != NULL){
			fputs(warning->line, stderr);
			if(warning->column != 0){
				for (unsigned int j = warning->column; j > 0; --j)
					fputc('.', stderr);
				fputs("^", stderr);
			}
			free(warning->line);
		}
		free(warning->message);
		free(warning->file_name);
		free(warning);
	}
}

int main(int argc, char **argv) {
	unsigned int i;
	unsigned ret;
	object_t *object;
	ret = 0;
	init_scas_runtime();
	scas_log_verbosity = L_ERROR;
	parse_arguments(argc, argv);
	validate_scas_runtime();
	instruction_set_t *instruction_set = find_inst();
	if (instruction_set == NULL) {
		scas_log(L_ERROR, "Failed to load instruction set definition, unable to continue!\n");
		return 1;
	}
	scas_log(L_INFO, "Loaded instruction set: %s", instruction_set->arch);
	list_t *include_path = split_include_path();
	list_t *errors = create_list();
	list_t *warnings = create_list();

	list_t *objects = create_list();
	assembler_settings_t settings = {
		.include_path = include_path,
		.set = instruction_set,
		.errors = errors,
		.warnings = warnings,
		.macros = scas_runtime.macros,
	};
	for (i = 0; i < scas_runtime.input_files->length; i += 1){
		object = assemble_input(scas_runtime.input_names->items[i], scas_runtime.input_files->items[i], settings);
		list_add(objects, object);
	}

	if(scas_runtime.link)
		link(objects, errors, warnings);
	else
		if(!merge(objects))
			ret = EXIT_FAILURE;

	handle_errors(errors, warnings);

	if(ret == 0)
		ret = errors->length;
	scas_log(L_DEBUG, "Exiting with status code %d, cleaning up", ret);
	// Remaining allocations:
	// scas_runtime.
	// 	input_files
	// 	input_names
	// include_path
	// objects, and all its items
	// errors
	// warnings
	// instruction_set
	// No point in freeing them, as the OS will do it faster anyways
	return ret;
}

