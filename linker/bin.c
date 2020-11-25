#include <stdio.h>
#include <stdint.h>
#include "bin.h"

int output_bin(FILE *f, uint8_t *data, int data_length) {
	return fwrite(data, sizeof(uint8_t), data_length, f) == (unsigned)data_length ? 0 : 1;
}
