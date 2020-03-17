#ifndef MUSX_H
#define MUSX_H

#include "mini_io.h"

typedef struct {
	char id[4];
	uint32_t length;
} iff_tag;

uint32_t decompress_musx_inner(mini_io_context *input, mini_io_context *output, iff_tag parent);
uint32_t decompress_musx_outer(mini_io_context *input, mini_io_context *output, iff_tag parent);
uint32_t decompress_musx(mini_io_context *input, mini_io_context *output);

#endif
