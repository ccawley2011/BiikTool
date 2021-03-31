#include "musx.h"
#include "compress.h"
#include "debug.h"
#include "mini_io.h"

extern void adpcm(mini_io_context * input, short * output, uint32_t insize);
unsigned char linear2ulaw(short pcm_val);

static unsigned char ulaw2vidc(unsigned char uval) {
	return ~(((uval >> 7) & 0x01) | ((uval << 1) & 0xFE));
}

static unsigned char linear2vidc(short pcm_val) {
	return ulaw2vidc(linear2ulaw(pcm_val));
}

iff_tag read_tag(mini_io_context *context) {
	iff_tag tag;
	MiniIO_Read(context, tag.id, 4, 1);
	tag.length = MiniIO_ReadLE32(context);
	return tag;
}

void write_tag(mini_io_context *context, iff_tag tag) {
	MiniIO_Write(context, tag.id, 4, 1);
	MiniIO_WriteLE32(context, tag.length);
}

uint32_t decompress_musx_comp_lzw(mini_io_context *input, mini_io_context *output, iff_tag comp) {
	off_t start = MiniIO_Tell(input) - 4;
	iff_tag tag = read_tag(input);
	uint32_t realsize;
	char *block = malloc(tag.length);
	if (!block)
		return 0;

	realsize = lzw_decode(input, block, tag.length);
	MiniIO_Seek(input, start + comp.length, MINI_IO_SEEK_SET);
	if (realsize != tag.length)
		warningf("Unexpected end of stream: expected %d bytes, got %d bytes", tag.length, realsize);

	write_tag(output, tag);
	MiniIO_Write(output, block, tag.length, 1);
	free(block);

	return tag.length + 8;
}

uint32_t decompress_musx_comp_null(mini_io_context *input, mini_io_context *output, iff_tag comp) {
	static const char blank_sample[] = {
		'S', 'A', 'M', 'P',
		 84,   0,   0,   0,
		'S', 'N', 'A', 'M',
		 20,   0,   0,   0,
		  0,   0,   0,   0,
		  0,   0,   0,   0,
		  0,   0,   0,   0,
		  0,   0,   0,   0,
		  0,   0,   0,   0,
		'S', 'V', 'O', 'L',
		  4,   0,   0,   0,
		  0,   0,   0,   0,
		'S', 'L', 'E', 'N',
		  4,   0,   0,   0,
		  0,   0,   0,   0,
		'R', 'O', 'F', 'S',
		  4,   0,   0,   0,
		  0,   0,   0,   0,
		'R', 'L', 'E', 'N',
		  4,   0,   0,   0,
		  0,   0,   0,   0,
		'S', 'D', 'A', 'T',
		  0,   0,   0,   0
	};
	/* Don't adjust the input... */
	(void)input;
	(void)comp;
	MiniIO_Write(output, blank_sample, sizeof(blank_sample), 1);
	return sizeof(blank_sample);
}

uint32_t decompress_musx_comp_adpcm(mini_io_context *input, mini_io_context *output, iff_tag comp) {
	off_t start = MiniIO_Tell(input) - 4;
	iff_tag tag = read_tag(input);
	uint32_t i, adpcm_size = comp.length - 12;
	short *slin_data = malloc (adpcm_size * 4);
	if (!slin_data)
		return 0;

	adpcm(input, slin_data, adpcm_size);
	MiniIO_Seek(input, start + comp.length, MINI_IO_SEEK_SET);

	write_tag(output, tag);
	for (i = 0; i < tag.length; i++) {
		MiniIO_WriteU8(output, linear2vidc(slin_data[i]));
	}
	free(slin_data);

	return tag.length + 8;
}

uint32_t decompress_musx_comp(mini_io_context *input, mini_io_context *output, iff_tag comp) {
	uint32_t type = MiniIO_ReadLE32(input);
	switch (type) {
	case 1: return decompress_musx_comp_lzw(input, output, comp);
	/* TODO: Support GSM decompression */
	case 3: return decompress_musx_comp_null(input, output, comp);
	case 4: return decompress_musx_comp_adpcm(input, output, comp);
	default:
		warningf("Unrecognised compression type %d", type);
		break;
	}

	write_tag(output, comp);
	MiniIO_WriteLE32(output, type);
	MiniIO_Copy(input, output, comp.length - 4, 1);
	return comp.length + 8;
}

uint32_t decompress_musx_inner(mini_io_context *input, mini_io_context *output, iff_tag parent) {
	off_t start = MiniIO_Tell(input);
	uint32_t size = 0;
	while (MiniIO_Tell(input) < (off_t)(start + parent.length)) {
		iff_tag tag = read_tag(input);
		if (strncmp(tag.id, "COMP", 4) == 0) {
			size += decompress_musx_comp(input, output, tag);
		} else if (strncmp(tag.id, "SAMP", 4) == 0) {
			size += decompress_musx_outer(input, output, tag);
		} else {
			write_tag(output, tag);
			MiniIO_Copy(input, output, tag.length, 1);
			size += tag.length + 8;
		}
	}
	return size;
}

uint32_t decompress_musx_outer(mini_io_context *input, mini_io_context *output, iff_tag parent) {
	off_t start = MiniIO_Tell(output);
	write_tag(output, parent);
	parent.length = decompress_musx_inner(input, output, parent);

	MiniIO_Seek(output, start, MINI_IO_SEEK_SET);
	write_tag(output, parent);
	MiniIO_Seek(output, parent.length, MINI_IO_SEEK_CUR);

	return parent.length + 8;
}

uint32_t decompress_musx(mini_io_context *input, mini_io_context *output) {
	iff_tag tag = read_tag(input);
	if (strncmp(tag.id, "MUSX", 4) != 0)
		return 0;

	return decompress_musx_outer(input, output, tag);
}

#ifdef TEST_MAIN
#define MINI_IO_IMPLEMENTATION
#include "mini_io.h"

int main(int argc, char **argv) {
	mini_io_context *input, *output;
	if (argc < 3) {
		fprintf(stderr, "Syntax: %s <input> <output>\n", argv[0]);
		return 1;
	}

	input = MiniIO_OpenFile(argv[1], MINI_IO_OPEN_READ);
	if (!input) {
		warningf("Could not open file %s", argv[1]);
		return 1;
	}

	output = MiniIO_OpenFile(argv[2], MINI_IO_OPEN_WRITE);
	if (!input) {
		warningf("Could not open file %s", argv[1]);
		MiniIO_DeleteContext(input);
		return 1;
	}

	decompress_musx(input, output);

	MiniIO_DeleteContext(output);
	MiniIO_DeleteContext(input);
	return 0;
}
#endif
