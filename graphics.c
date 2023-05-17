#include "graphics.h"
#include "compress.h"
#include "utils.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

static uint32_t decompress_frame_lzw(FILE *input, unsigned char **sprite, uint32_t *spritesize) {
	long start = ftell(input);
	uint32_t size = lzw_decode(input, NULL, 0);
	fseek(input, start, SEEK_SET);

	if (!*sprite) {
		*sprite = malloc(size);
		*spritesize = size;
		if (!*sprite)
			return 0;
	} else if (*spritesize < size) {
		unsigned char *newsprite = realloc(*sprite, size);
		if (newsprite) {
			*sprite = newsprite;
			*spritesize = size;
		} else {
			size = *spritesize;
		}
	}

	return lzw_decode(input, *sprite, size);
}


static int read_graphic_header(FILE *input, graphic_header *header) {
	int be = 0;
	unsigned char buf[32];
	long start = ftell(input);

	fread(buf, 32, 1, input);
	be = (buf[3] != 0);

	header->index   = read_u32(buf + 0,  be);
	header->frames  = read_u32(buf + 4,  be);
	header->mode    = read_u32(buf + 8,  be);
	header->xdiv    = read_u32(buf + 12, be);
	header->ydiv    = read_u32(buf + 16, be);
	header->width   = read_u32(buf + 20, be);
	header->height  = read_u32(buf + 24, be);
	header->palsize = read_u32(buf + 28, be);
	/* TODO: header->palette */

	fseek(input, start + header->index, SEEK_SET);
	fread(buf, 16, 1, input);

	header->unknown1 = read_u32(buf + 0,  be);
	header->count    = read_u32(buf + 4,  be);
	header->unknown2 = read_u32(buf + 8,  be);
	header->unknown3 = read_u32(buf + 12, be);
	/* TODO: header->unknown4 */

	fseek(input, start + header->frames, SEEK_SET);

	return be;
}

static void read_sprite_area(const unsigned char *buf, sprite_area *area, int is_file) {
	if (!is_file) {
		area->size = read_u32(buf + 0, 0);
		buf += 4;
	}
	area->count = read_u32(buf + 0, 0);
	area->start = read_u32(buf + 4, 0);
	area->end   = read_u32(buf + 8, 0);
}

static void write_sprite_area(unsigned char *buf, sprite_area *area, int is_file) {
	if (!is_file) {
		write_u32(buf + 0, area->size, 0);
		buf += 4;
	}
	write_u32(buf + 0, area->count, 0);
	write_u32(buf + 4, area->start, 0);
	write_u32(buf + 8, area->end,   0);
}

static uint32_t append_sprite(FILE *output, unsigned char *sprite,  sprite_area *area) {
	sprite_area input_area;
	uint32_t size, i;
	long start, pos = 0;

	start = ftell(output);
	read_sprite_area(sprite, &input_area, 0);
	size = input_area.end - input_area.start;
	fwrite(sprite + 16, size, 1, output);
	area->end += size;

	for (i = 0; i < input_area.count; i++) {
		char name[12];
		memset(name, 0, sizeof(name));
		snprintf(name, 12, "%d", area->count++);

		fseek(output, start + pos + 4, SEEK_SET);
		fwrite(name, 12, 1, output);

		pos += read_u32(sprite + 16 + pos, 0);
	}

	fseek(output, 0, SEEK_END);

	return size;
}

uint32_t decompress_graphic(FILE *input, FILE *output) {
	unsigned char buf[24];
	graphic_header header;
	sprite_area area;
	int be;
	unsigned char *sprite = NULL;
	uint32_t spritesize = 0, total = 12, i;

	area.count = 0;
	area.start = 16;
	area.end   = 16;
	write_sprite_area(buf, &area, 1);
	fwrite(buf, 12, 1, output);

	be = read_graphic_header(input, &header);

	for (i = 0; i < header.count; i++) {
		long start = ftell(input);
		graphic_frame frame;

		fread(buf, 24, 1, input);
		frame.size     = read_u32(buf + 0,  be);
		frame.method   = read_u32(buf + 4,  be);
		frame.unknown1 = read_u32(buf + 8,  be);
		frame.unknown2 = read_u32(buf + 12, be);
		frame.unknown3 = read_u32(buf + 16, be);
		frame.unknown4 = read_u32(buf + 20, be);

		if (frame.method == 2) {
			decompress_frame_lzw(input, &sprite, &spritesize);
		} else {
			warningf("Unrecognised compression method %d",
			         frame.method);
			break;
		}
		if (sprite) {
			total += append_sprite(output, sprite, &area);
		}

		fseek(input, start + frame.size, SEEK_SET);
	}

	fseek(output, 0, SEEK_SET);
	write_sprite_area(buf, &area, 1);
	fwrite(buf, 12, 1, output);

	free(sprite);
	return total;
}
