typedef struct linker_settings linker_settings_t;

typedef int (*format_writer)(FILE *f, object_t *object, linker_settings_t *settings);

struct linker_settings {
    int automatic_relocation;
    list_t *errors;
    list_t *warnings;
    format_writer write_output;
};

void link_objects(FILE *output, list_t *objects, linker_settings_t *settings);
