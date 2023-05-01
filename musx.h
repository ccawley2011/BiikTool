#ifndef MUSX_H
#define MUSX_H

#include <stdint.h>
#include <stdio.h>

typedef enum {
	tag_MUSX = 0x5853554D,
	tag_SAMP = 0x504D4153,
	tag_COMP = 0x504D4F43
} iff_tag_id;

typedef struct {
	uint32_t id, length;
} iff_tag;

uint32_t decompress_musx_inner(FILE *input, FILE *output, iff_tag parent);
uint32_t decompress_musx_outer(FILE *input, FILE *output, iff_tag parent);
uint32_t decompress_musx(FILE *input, FILE *output);

#endif
