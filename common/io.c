#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include "io.h"
#include "log.h"

void scas_read(void *ptr, size_t size, size_t nmemb, FILE *stream) {
	const unsigned read = fread(ptr, size, nmemb, stream);
	if (read != nmemb) {
		scas_abort("File read returned %d, %d expected", read * size, nmemb * size);
	}
}
