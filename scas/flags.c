#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#define _BSD_EXTENSION
#include <ctype.h>

#include "stack.h"
#include "list.h"
#include "enums.h"
#include "expression.h"
#include "log.h"
#include "linker.h"
#include "runtime.h"
#include "bin.h"
#include "8xp.h"
#include "flags.h"

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
		struct output_format o = { .name=value };
		struct output_format *res =
			bsearch(&o, output_formats,
				sizeof(output_formats) / sizeof(struct output_format),
				sizeof(struct output_format), format_compare);
		if (!res) {
			scas_log(L_ERROR, "Unknown output format %s", value);
			return false;
		}
		scas_runtime.options.output_format = res->writer;
		scas_runtime.output_extension = res->name;
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
