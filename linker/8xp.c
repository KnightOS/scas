#include "8xp.h"
#include <stdio.h>
#include <stdint.h>

int output_8xp(FILE *f, uint8_t *data, int data_length) {
	/* TODO: Implement this */
	fwrite(data, sizeof(uint8_t), data_length, f);
	return 0;
}
