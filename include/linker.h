typedef int (*format_writer)(FILE *f, uint8_t *data, int data_length);

typedef struct {
    int automatic_relocation;
    int merge_only;
    list_t *errors;
    list_t *warnings;
    format_writer write_output;
} linker_settings_t;

void link_objects(FILE *output, list_t *objects, linker_settings_t *settings);
