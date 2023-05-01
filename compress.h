#ifndef COMPRESS_H
#define COMPRESS_H

#include <stdio.h>

uint32_t lzw_decode(FILE *in, char *out, uint32_t outsize);

#endif
