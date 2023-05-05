#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdint.h>
#include <stdio.h>

typedef struct {
    uint32_t index;
    uint32_t frames;
    uint32_t mode;
    uint32_t xdiv;
    uint32_t ydiv;
    uint32_t width;
    uint32_t height;
    uint32_t palsize;

    uint32_t unknown1;
    uint32_t count;
    uint32_t unknown2;
    uint32_t unknown3;
} graphic_header;

typedef struct {
    uint32_t size;
    uint32_t method;
    uint32_t unknown1;
    uint32_t unknown2;
    uint32_t unknown3;
    uint32_t unknown4;
} graphic_frame;

typedef struct {
    uint32_t size;
    uint32_t count;
    uint32_t start;
    uint32_t end;
} sprite_area;

uint32_t decompress_graphic(FILE *input, FILE *output);

#endif
