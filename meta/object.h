#ifndef OBJECT_H
#define OBJECT_H

enum {
	Z80 = 0,
	LR35902 = 1
};

typedef struct {
	uint8_t exported;
	uint32_t name_length;
	char *name;
	uint32_t value;
} sass_symbol_t;

enum {
	UNSIGNED_16_BIT = 0,
	UNSIGNED_8_BIT = 1,
	SIGNED_8_BIT = 2
};

typedef struct {
	uint8_t type;
	uint32_t address;
	uint32_t expression_length;
	char *expression;
} sass_expression_t;

typedef struct {
	uint32_t address;
	uint32_t output_length;
	uint32_t code_length;
	char *code;
} sass_line_t;

typedef struct {
	uint8_t version;
	uint8_t architecture;

	uint32_t symbol_count;
	sass_symbol_t *symbols;

	uint32_t machine_code_length;
	uint8_t *machine_code;

	uint32_t expression_count;
	sass_expression_t *expressions;

	uint32_t line_count;
	sass_line_t *lines;
} sass_object_t;

void write_object_to_file(sass_object_t object, FILE *file);

#endif
