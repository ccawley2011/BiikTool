#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <stdio.h>

#ifdef __GNUC__
# define PRINTF_ATTR(x,y) __attribute__((__format__(__printf__, x, y)))
#else
# define PRINTF_ATTR(x,y)
#endif

void warning(const char *message);
void warningf(const char *message, ...) PRINTF_ATTR(1,2);


size_t fcopy(FILE *input, FILE *output, size_t size);
long fsize(FILE *context);

uint32_t read_u32(unsigned char *buf, int be);
void write_u32(unsigned char *buf, uint32_t val, int be);

#endif
