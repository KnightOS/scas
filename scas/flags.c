#include "flags.h"
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <ctype.h>
#include "runtime.h"
#include "log.h"
#include "linker.h"
#include "bin.h"
#include "8xp.h"

/* Keep alphabetized */
struct output_format output_formats[] = {
	{ "bin", output_bin },
	{ "8xp", output_8xp }
};

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
