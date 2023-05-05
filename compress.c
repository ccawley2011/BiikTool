#include <assert.h>
#include <stdio.h>
#include <stdint.h>

#include "compress.h"

/*
 * LZW decompression based on http://ttf.mine.nu/techdocs/unpack.pl
 */

enum {
	SET_BITS = 8,
	MAX_BITS = 12,

	CLEAR_CODE = (1 << SET_BITS),
	END_CODE = CLEAR_CODE + 1,
	FIRST_CODE = CLEAR_CODE + 2,

	MAX_TABLE = (1 << MAX_BITS)
};

struct lzw_dict {
	uint16_t prefix;
	unsigned char postfix, first_byte;
};

#define LZW_FIRST_BYTE(x, y) (char)(x < FIRST_CODE ? x : y[x - FIRST_CODE].first_byte)

uint32_t lzw_decode(FILE *in, unsigned char *out, uint32_t outsize) {
	struct lzw_dict dict[MAX_TABLE];
	unsigned int bitpos = 0, nbit = SET_BITS + 1;
	uint16_t dictsize = FIRST_CODE;
	uint16_t prev = CLEAR_CODE;
	uint32_t buf24 = ((fgetc(in) << 16) | (fgetc(in) << 8) | fgetc(in));
	uint32_t size = 0;

	while (prev != END_CODE) {
		uint16_t cw;
		if (prev == CLEAR_CODE) {
			nbit = SET_BITS + 1;
			dictsize = FIRST_CODE;
		}

		/* Get next codeword */
		bitpos += nbit;
		cw = (buf24 >> (24-bitpos)) & ((1 << nbit) - 1);
		buf24 = (buf24 << 8) + fgetc(in);
		if (bitpos >= 16) {
			buf24 = (buf24 << 8) + fgetc(in);
		}
		buf24 &= 0xFFFFFF;
		bitpos &= 7;

		/* Process the codeword cw */
		if (cw != CLEAR_CODE && cw != END_CODE) {
			uint16_t outcw;
			int i, j = 0;
			unsigned char temp[1024];
			unsigned char newbyte;
			if (cw < dictsize) {
				newbyte = LZW_FIRST_BYTE(cw, dict);
			} else {
				assert(prev != CLEAR_CODE);
				assert(dictsize < MAX_TABLE);
				assert(cw == dictsize);
				newbyte = LZW_FIRST_BYTE(prev, dict);
			}

			if ((prev != CLEAR_CODE) && (dictsize < MAX_TABLE)) {
				struct lzw_dict *d = &dict[dictsize - FIRST_CODE];
				d->prefix = prev;
				d->postfix = newbyte;
				d->first_byte = LZW_FIRST_BYTE(prev,dict);
				dictsize++;

				if ((dictsize == (1 << nbit)) && (nbit < MAX_BITS)) {
					nbit++;
				}
			}

			/* write the sequence here */
			outcw = cw;
			while (outcw >= FIRST_CODE) {
				struct lzw_dict *d = &dict[outcw - FIRST_CODE];
				outcw = d->prefix;
				temp[j++] = d->postfix;
			}
			temp[j++] = (unsigned char)outcw;

			if (out) {
				for (i = j; i > 0; i--) {
					out[size++] = temp[i-1];
					if (size >= outsize)
						return size;
				}
			} else {
				size += j;
			}
		}
		prev = cw;
	}

	return size;
}
