#ifndef COMPRESS_H
#define COMPRESS_H

#include "mini_io.h"

uint32_t lzw_decode(mini_io_context *in, char *out, uint32_t outsize);

#endif
