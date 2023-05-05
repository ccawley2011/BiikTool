#include "utils.h"

#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>

void warning(const char *message) {
	fputs("WARNING: ", stderr);
	fputs(message, stderr);
	fputc('\n', stderr);
}

void warningf(const char *message, ...) {
	va_list args;

	fputs("WARNING: ", stderr);

	va_start(args, message);
	vfprintf(stderr, message, args);
	va_end(args);

	fputc('\n', stderr);
}


size_t fcopy(FILE *input, FILE *output, size_t size) {
	char *block;
	assert(input);
	assert(output);
	block = malloc(size);
	if (!block)
		return 0;

	if (fread(block, size, 1, input) == 0)
		return 0;
	if (fwrite(block, size, 1, output) == 0)
		return 0;

	free(block);
	return size;
}

long fsize(FILE *context) {
	long oldpos, length;

	oldpos = ftell(context);
	fseek(context, 0, SEEK_END);
	length = ftell(context);
	fseek(context, oldpos, SEEK_SET);

	return length;
}


uint32_t read_u32(const unsigned char *buf, int be) {
	uint32_t val = 0;
	if (be) {
		val  = buf[3];
		val |= buf[2] << 8;
		val |= buf[1] << 16;
		val |= buf[0] << 24;
	} else {
		val  = buf[0];
		val |= buf[1] << 8;
		val |= buf[2] << 16;
		val |= buf[3] << 24;
	}
	return val;
}

void write_u32(unsigned char *buf, uint32_t val, int be) {
	if (be) {
		buf[3] = (val >> 0) & 0xFF;
		buf[2] = (val >> 8) & 0xFF;
		buf[1] = (val >> 16) & 0xFF;
		buf[0] = (val >> 24) & 0xFF;
	} else {
		buf[0] = (val >> 0) & 0xFF;
		buf[1] = (val >> 8) & 0xFF;
		buf[2] = (val >> 16) & 0xFF;
		buf[3] = (val >> 24) & 0xFF;
	}
}
