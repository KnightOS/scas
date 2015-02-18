#include "functions.h"
#include "list.h"
#include "log.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <strings.h>

list_t *decode_function_metadata(char *value, uint64_t value_length) {
	uint32_t total;
	list_t *result;
	total = *(uint32_t *)value;
	value += sizeof(uint32_t);
	result = create_list();

	int i;
	for (i = 0; i < total; ++i) {
		uint32_t len;
		function_metadata_t *meta = malloc(sizeof(function_metadata_t));
		len = *(uint32_t *)value;
		value += sizeof(uint32_t);
		meta->name = malloc(len + 1);
		strcpy(meta->name, value);
		value += len + 1;

		len = *(uint32_t *)value;
		value += sizeof(uint32_t);
		meta->start_symbol = malloc(len + 1);
		strcpy(meta->start_symbol, value);
		value += len + 1;

		len = *(uint32_t *)value;
		value += sizeof(uint32_t);
		meta->end_symbol = malloc(len + 1);
		strcpy(meta->end_symbol, value);
		value += len + 1;

		list_add(result, meta);
	}
	return result;
}

char *encode_function_metadata(list_t *metadata, uint64_t *value_length) {
	uint32_t len = sizeof(uint32_t);
	int i;
	for (i = 0; i < metadata->length; ++i) {
		function_metadata_t *meta = metadata->items[i];
		len += strlen(meta->name) + 1 + sizeof(uint32_t);
		len += strlen(meta->start_symbol) + 1 + sizeof(uint32_t);
		len += strlen(meta->end_symbol) + 1 + sizeof(uint32_t);
	}
	char *result = malloc(len);
	scas_log(L_DEBUG, "Allocated %d bytes for function metadata", len);
	*value_length = (uint32_t)len;
	*(uint32_t *)result = (uint32_t)metadata->length;
	int ptr = sizeof(uint32_t);
	for (i = 0; i < metadata->length; ++i) {
		function_metadata_t *meta = metadata->items[i];

		*(uint32_t *)(result + ptr) = (uint32_t)strlen(meta->name);
		ptr += sizeof(uint32_t);
		strcpy(result + ptr, meta->name);
		ptr += strlen(meta->name) + 1;

		*(uint32_t *)(result + ptr) = (uint32_t)strlen(meta->start_symbol);
		ptr += sizeof(uint32_t);
		strcpy(result + ptr, meta->start_symbol);
		ptr += strlen(meta->start_symbol) + 1;
		
		*(uint32_t *)(result + ptr) = (uint32_t)strlen(meta->end_symbol);
		ptr += sizeof(uint32_t);
		strcpy(result + ptr, meta->end_symbol);
		ptr += strlen(meta->end_symbol) + 1;
	}
	return result;
}
