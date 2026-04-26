#ifndef COMPRESS_H
#define COMPRESS_H

#include <stdio.h>

uint32_t lzw_decode(FILE *in, unsigned char *out, uint32_t outsize);
void rlexor_decode(const unsigned char *in, uint32_t insize,
                   unsigned char *out, uint32_t outsize);

#endif
