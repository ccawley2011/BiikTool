#include <assert.h>
#include <stdint.h>

#include "archive.h"
#include "compress.h"
#include "debug.h"
#include "extract.h"
#include "mini_io.h"

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
	int prefix;
	char postfix, first_byte;
};

#define LZW_FIRST_BYTE(x, y) (char)(x < FIRST_CODE ? x : y[x - FIRST_CODE].first_byte)

uint32_t lzw_decode(mini_io_context *in, char *out, uint32_t outsize) {
	struct lzw_dict dict[MAX_TABLE];
	unsigned int bitpos = 0, nbit = SET_BITS + 1;
	int dictsize = FIRST_CODE;
	int prev = CLEAR_CODE;
	uint32_t buf24 = ((MiniIO_ReadU8(in) << 16) | (MiniIO_ReadU8(in) << 8) | MiniIO_ReadU8(in));
	uint32_t size = 0;

	while (prev != END_CODE) {
		int cw;
		if (prev == CLEAR_CODE) {
			nbit = SET_BITS + 1;
			dictsize = FIRST_CODE;
		}

		/* Get next codeword */
		bitpos += nbit;
		cw = (buf24 >> (24-bitpos)) & ((1 << nbit) - 1);
		buf24 = (buf24 << 8) + MiniIO_ReadU8(in);
		if (bitpos >= 16) {
			buf24 = (buf24 << 8) + MiniIO_ReadU8(in);
		}
		buf24 &= 0xFFFFFF;
		bitpos &= 7;

		/* Process the codeword cw */
		if (cw != CLEAR_CODE && cw != END_CODE) {
			int outcw, i, j = 0;
			char temp[1024];
			char newbyte;
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
			temp[j++] = (char)outcw;

			for (i = j; i > 0; i--) {
				out[size++] = temp[i-1];
				if (size > outsize)
					return size;
			}
		}
		prev = cw;
	}

	return size;
}
