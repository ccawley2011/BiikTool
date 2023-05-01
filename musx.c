#include "musx.h"
#include "compress.h"
#include "utils.h"

#include <stdlib.h>

extern void adpcm(FILE * input, short * output, uint32_t insize);
unsigned char linear2ulaw(short pcm_val);

static unsigned char ulaw2vidc(unsigned char uval) {
	return ~(((uval >> 7) & 0x01) | ((uval << 1) & 0xFE));
}

static unsigned char linear2vidc(short pcm_val) {
	return ulaw2vidc(linear2ulaw(pcm_val));
}

iff_tag read_tag(FILE *in) {
	iff_tag tag;
	unsigned char buf[8];

	fread(buf, 8, 1, in);
	tag.id = read_u32(buf + 0, 0);
	tag.length = read_u32(buf + 4, 0);
	return tag;
}

void write_tag(FILE *out, iff_tag tag) {
	unsigned char buf[8];

	write_u32(buf + 0, tag.id, 0);
	write_u32(buf + 4, tag.length, 0);
	fwrite(buf, 8, 1, out);
}

uint32_t decompress_musx_comp_lzw(FILE *input, FILE *output, iff_tag comp) {
	long start = ftell(input) - 4;
	iff_tag tag = read_tag(input);
	uint32_t realsize;
	char *block = malloc(tag.length);
	if (!block)
		return 0;

	realsize = lzw_decode(input, block, tag.length);
	fseek(input, start + comp.length, SEEK_SET);
	if (realsize != tag.length)
		warningf("Unexpected end of stream: expected %d bytes, got %d bytes", tag.length, realsize);

	write_tag(output, tag);
	fwrite(block, tag.length, 1, output);
	free(block);

	return tag.length + 8;
}

uint32_t decompress_musx_comp_null(FILE *input, FILE *output, iff_tag comp) {
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
	fwrite(blank_sample, sizeof(blank_sample), 1, output);
	return sizeof(blank_sample);
}

uint32_t decompress_musx_comp_adpcm(FILE *input, FILE *output, iff_tag comp) {
	long start = ftell(input) - 4;
	iff_tag tag = read_tag(input);
	uint32_t i, adpcm_size = comp.length - 12;
	short *slin_data = malloc (adpcm_size * 4);
	if (!slin_data)
		return 0;

	adpcm(input, slin_data, adpcm_size);
	fseek(input, start + comp.length, SEEK_SET);

	write_tag(output, tag);
	for (i = 0; i < tag.length; i++) {
		fputc(linear2vidc(slin_data[i]), output);
	}
	free(slin_data);

	return tag.length + 8;
}

uint32_t decompress_musx_comp(FILE *input, FILE *output, iff_tag comp) {
	uint32_t type;
	unsigned char buf[4];

	fread(buf, 4, 1, input);
	type = read_u32(buf, 0);

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
	fwrite(buf, 4, 1, output);
	fcopy(input, output, comp.length - 4);
	return comp.length + 8;
}

uint32_t decompress_musx_inner(FILE *input, FILE *output, iff_tag parent) {
	long start = ftell(input);
	uint32_t size = 0;
	while (ftell(input) < (long)(start + parent.length)) {
		iff_tag tag = read_tag(input);
		if (tag.id == tag_COMP) {
			size += decompress_musx_comp(input, output, tag);
		} else if (tag.id == tag_SAMP) {
			size += decompress_musx_outer(input, output, tag);
		} else {
			write_tag(output, tag);
			fcopy(input, output, tag.length);
			size += tag.length + 8;
		}
	}
	return size;
}

uint32_t decompress_musx_outer(FILE *input, FILE *output, iff_tag parent) {
	long start = ftell(output);
	write_tag(output, parent);
	parent.length = decompress_musx_inner(input, output, parent);

	fseek(output, start, SEEK_SET);
	write_tag(output, parent);
	fseek(output, parent.length, SEEK_CUR);

	return parent.length + 8;
}

uint32_t decompress_musx(FILE *input, FILE *output) {
	iff_tag tag = read_tag(input);
	if (tag.id != tag_MUSX)
		return 0;

	return decompress_musx_outer(input, output, tag);
}

#ifdef TEST_MAIN
int main(int argc, char **argv) {
	FILE *input, *output;
	if (argc < 3) {
		fprintf(stderr, "Syntax: %s <input> <output>\n", argv[0]);
		return 1;
	}

	input = fopen(argv[1], "rb");
	if (!input) {
		warningf("Could not open file %s", argv[1]);
		return 1;
	}

	output = fopen(argv[2], "wb");
	if (!input) {
		warningf("Could not open file %s", argv[1]);
		fclose(input);
		return 1;
	}

	decompress_musx(input, output);

	fclose(output);
	fclose(input);
	return 0;
}
#endif
