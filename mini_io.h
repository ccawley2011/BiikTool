/*
 * TODO:
 *  - More implementations
 */

#ifndef MINI_IO_H
#define MINI_IO_H

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* TODO: Use our own types? */
#ifdef _MSC_VER
typedef __int64 off_t;
#else
#include <sys/types.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct mini_io_context mini_io_context;

typedef struct mini_io_callbacks {
	size_t (*read)(mini_io_context *context, void *ptr, size_t size, size_t maxnum);
	size_t (*write)(mini_io_context *context, const void *ptr, size_t size, size_t num);
	void (*seek)(mini_io_context *context, off_t n, int whence);
	off_t (*tell)(mini_io_context *context);
	off_t (*size)(mini_io_context *context);
	int (*eof)(mini_io_context *context);
	int (*flush)(mini_io_context *context);
	int (*close)(mini_io_context *context);
} mini_io_callbacks;

struct mini_io_context {
	const mini_io_callbacks *callbacks;
	void *data;

	int autoclose;
	int readonly;
};

enum {
#ifdef MINI_IO_DISABLE_STDIO
	MINI_IO_SEEK_SET,
	MINI_IO_SEEK_CUR,
	MINI_IO_SEEK_END
#else
	MINI_IO_SEEK_SET = SEEK_SET,
	MINI_IO_SEEK_CUR = SEEK_CUR,
	MINI_IO_SEEK_END = SEEK_END
#endif
};

enum {
	MINI_IO_OPEN_READ,
	MINI_IO_OPEN_WRITE,
	MINI_IO_OPEN_APPEND,

	MINI_IO_OPEN_UPDATE = 1 << 8
};

size_t MiniIO_Read(mini_io_context *context, void *ptr, size_t size, size_t maxnum);
size_t MiniIO_Write(mini_io_context *context, const void *ptr, size_t size, size_t num);
void MiniIO_Seek(mini_io_context *context, off_t n, int whence);
off_t MiniIO_Tell(mini_io_context *context);
off_t MiniIO_Size(mini_io_context *context);
int MiniIO_EOF(mini_io_context *context);
int MiniIO_Flush(mini_io_context *context);

size_t MiniIO_Copy(mini_io_context *input, mini_io_context *output, size_t size, size_t maxnum);

uint8_t MiniIO_ReadU8(mini_io_context *context);
uint16_t MiniIO_ReadLE16(mini_io_context *context);
uint16_t MiniIO_ReadBE16(mini_io_context *context);
uint16_t MiniIO_ReadU16(mini_io_context *context, int endian);
uint32_t MiniIO_ReadLE32(mini_io_context *context);
uint32_t MiniIO_ReadBE32(mini_io_context *context);
uint32_t MiniIO_ReadU32(mini_io_context *context, int endian);
uint64_t MiniIO_ReadLE64(mini_io_context *context);
uint64_t MiniIO_ReadBE64(mini_io_context *context);
uint64_t MiniIO_ReadU64(mini_io_context *context, int endian);

size_t MiniIO_WriteU8(mini_io_context *context, uint8_t value);
size_t MiniIO_WriteLE16(mini_io_context *context, uint16_t value);
size_t MiniIO_WriteBE16(mini_io_context *context, uint16_t value);
size_t MiniIO_WriteU16(mini_io_context *context, uint16_t value, int endian);
size_t MiniIO_WriteLE32(mini_io_context *context, uint32_t value);
size_t MiniIO_WriteBE32(mini_io_context *context, uint32_t value);
size_t MiniIO_WriteU32(mini_io_context *context, uint32_t value, int endian);
size_t MiniIO_WriteLE64(mini_io_context *context, uint64_t value);
size_t MiniIO_WriteBE64(mini_io_context *context, uint64_t value);
size_t MiniIO_WriteU64(mini_io_context *context, uint64_t value, int endian);

mini_io_context *MiniIO_CreateContext(mini_io_callbacks *callbacks, void *data);
int MiniIO_DeleteContext(mini_io_context *context);

mini_io_context *MiniIO_OpenFile(const char *filename, int mode);
#ifndef MINI_IO_DISABLE_STDIO
mini_io_context *MiniIO_CreateFromFP(FILE *stream, int autoclose);
#endif
mini_io_context *MiniIO_CreateFromMem(void *mem, size_t size);
mini_io_context *MiniIO_CreateFromConstMem(const void *mem, size_t size);
mini_io_context *MiniIO_CreateFromContext(mini_io_context *parent, off_t start, off_t size, int autoclose, int safe);

#ifdef __GNUC__
# define MINI_IO_GCC_ATLEAST(x,y) ((__GNUC__ > x) || (__GNUC__ == x && __GNUC_MINOR__ >= y))
#else
# define MINI_IO_GCC_ATLEAST(x,y) 0
#endif

#ifdef __has_builtin
# define MINI_IO_HAS_BUILTIN(x) __has_builtin(x)
#else
# define MINI_IO_HAS_BUILTIN(x) 0
#endif

#ifndef MINI_IO_BSWAP16
# if defined(_MSC_VER) && _MSC_VER >= 1300
#  define MINI_IO_BSWAP16(x) _byteswap_ushort(x)
# elif MINI_IO_GCC_ATLEAST(4, 8) || MINI_IO_HAS_BUILTIN(__builtin_bswap16)
#  define MINI_IO_BSWAP16(x) __builtin_bswap16(x)
# else
#  define MINI_IO_BSWAP16(x) ((uint16_t) ((x << 8) | (x >> 8)))
# endif
#endif

#ifndef MINI_IO_BSWAP32
# if defined(_MSC_VER) && _MSC_VER >= 1300
#  define MINI_IO_BSWAP32(x) _byteswap_ulong(x)
# elif MINI_IO_GCC_ATLEAST(4, 3) || MINI_IO_HAS_BUILTIN(__builtin_bswap32)
#  define MINI_IO_BSWAP32(x) __builtin_bswap32(x)
# else
#  define MINI_IO_BSWAP32(x) ((uint32_t) ((((x) >> 24) & 0x00FF) | \
                                          (((x) >>  8) & 0xFF00) | \
                                          (((x) & 0xFF00) <<  8) | \
                                          (((x) & 0x00FF) << 24) ) )
# endif
#endif

#ifndef MINI_IO_BSWAP64
# if defined(_MSC_VER) && _MSC_VER >= 1300
#  define MINI_IO_BSWAP64(x) _byteswap_uint64(x)
# elif MINI_IO_GCC_ATLEAST(4, 3) || MINI_IO_HAS_BUILTIN(__builtin_bswap64)
#  define MINI_IO_BSWAP64(x) __builtin_bswap64(x)
# else
#  define MINI_IO_BSWAP64(x) ((uint64_t) ((((x) >> 56) & 0x000000FF) | \
                                          (((x) >> 40) & 0x0000FF00) | \
                                          (((x) >> 24) & 0x00FF0000) | \
                                          (((x) >>  8) & 0xFF000000) | \
                                          (((x) & 0xFF000000) <<  8) | \
                                          (((x) & 0x00FF0000) << 24) | \
                                          (((x) & 0x0000FF00) << 40) | \
                                          (((x) & 0x000000FF) << 56) ) )
# endif
#endif

#define MINI_IO_LITTLE_ENDIAN 1234
#define MINI_IO_BIG_ENDIAN    4321

#if !defined(MINI_IO_ENDIAN)
/* TODO: Autodetect this! */
# define MINI_IO_ENDIAN MINI_IO_LITTLE_ENDIAN
#endif

#if MINI_IO_ENDIAN == MINI_IO_LITTLE_ENDIAN
# define MINI_IO_BSWAP16_LE(x) (x)
# define MINI_IO_BSWAP32_LE(x) (x)
# define MINI_IO_BSWAP64_LE(x) (x)
# define MINI_IO_BSWAP16_BE(x) MINI_IO_BSWAP16(x)
# define MINI_IO_BSWAP32_BE(x) MINI_IO_BSWAP32(x)
# define MINI_IO_BSWAP64_BE(x) MINI_IO_BSWAP64(x)
#elif MINI_IO_ENDIAN == MINI_IO_BIG_ENDIAN
# define MINI_IO_BSWAP16_LE(x) MINI_IO_BSWAP16(x)
# define MINI_IO_BSWAP32_LE(x) MINI_IO_BSWAP32(x)
# define MINI_IO_BSWAP64_LE(x) MINI_IO_BSWAP64(x)
# define MINI_IO_BSWAP16_BE(x) (x)
# define MINI_IO_BSWAP32_BE(x) (x)
# define MINI_IO_BSWAP64_BE(x) (x)
#else
# error Unknown endianness
#endif

#ifndef MINI_IO_CLAMP
# define MINI_IO_CLAMP(x, min, max) ((x < min) ? min : ((x > max) ? max : x))
#endif
#ifndef MINI_IO_UNUSED
# define MINI_IO_UNUSED(x) (void)x
#endif

#ifdef __cplusplus
}
#endif

#endif

/***************************************/

#ifdef MINI_IO_IMPLEMENTATION

size_t MiniIO_Read(mini_io_context *context, void *ptr, size_t size, size_t maxnum) {
	assert(context);
	return context->callbacks->read(context, ptr, size, maxnum);
}

size_t MiniIO_Write(mini_io_context *context, const void *ptr, size_t size, size_t num) {
	assert(context);
	return context->callbacks->write(context, ptr, size, num);
}

void MiniIO_Seek(mini_io_context *context, off_t n, int whence) {
	assert(context);
	context->callbacks->seek(context, n, whence);
}

off_t MiniIO_Tell(mini_io_context *context) {
	assert(context);
	return context->callbacks->tell(context);
}

off_t MiniIO_Size(mini_io_context *context) {
	assert(context);
	return context->callbacks->size(context);
}

int MiniIO_EOF(mini_io_context *context) {
	assert(context);
	return context->callbacks->eof(context);
}

int MiniIO_Flush(mini_io_context *context) {
	assert(context);
	return context->callbacks->flush(context);
}

/***************************************/

size_t MiniIO_Copy(mini_io_context *input, mini_io_context *output, size_t size, size_t maxnum) {
	size_t i, read = 0;
	char *block;
	assert(input);
	assert(output);
	block = malloc(size);
	if (!block)
		return 0;

	for (i = 0; i < maxnum; i++) {
		if (MiniIO_Read(input, block, size, 1) == 0)
			break;    
		if (MiniIO_Write(output, block, size, 1) == 0)
			break;
		read++;
	}

	free(block);
	return read;
}

uint8_t MiniIO_ReadU8(mini_io_context *context) {
	uint8_t value = 0;
	MiniIO_Read(context, &value, sizeof(value), 1);
	return value;
}

uint16_t MiniIO_ReadLE16(mini_io_context *context) {
	uint16_t value = 0;
	MiniIO_Read(context, &value, sizeof(value), 1);
	return MINI_IO_BSWAP16_LE(value);
}

uint16_t MiniIO_ReadBE16(mini_io_context *context) {
	uint16_t value = 0;
	MiniIO_Read(context, &value, sizeof(value), 1);
	return MINI_IO_BSWAP16_BE(value);
}

uint16_t MiniIO_ReadU16(mini_io_context *context, int endian) {
	uint16_t value = 0;
	MiniIO_Read(context, &value, sizeof(value), 1);
	if (endian == MINI_IO_BIG_ENDIAN) {
		return MINI_IO_BSWAP16_BE(value);
	} else if (endian == MINI_IO_LITTLE_ENDIAN) {
		 return MINI_IO_BSWAP16_LE(value);
	} else {
		assert(0);
		return value;
	}
}

uint32_t MiniIO_ReadLE32(mini_io_context *context) {
	uint32_t value = 0;
	MiniIO_Read(context, &value, sizeof(value), 1);
	return MINI_IO_BSWAP32_LE(value);
}

uint32_t MiniIO_ReadBE32(mini_io_context *context) {
	uint32_t value = 0;
	MiniIO_Read(context, &value, sizeof(value), 1);
	return MINI_IO_BSWAP32_BE(value);
}

uint32_t MiniIO_ReadU32(mini_io_context *context, int endian) {
	uint32_t value = 0;
	MiniIO_Read(context, &value, sizeof(value), 1);
	if (endian == MINI_IO_BIG_ENDIAN) {
		return MINI_IO_BSWAP32_BE(value);
	} else if (endian == MINI_IO_LITTLE_ENDIAN) {
		 return MINI_IO_BSWAP32_LE(value);
	} else {
		assert(0);
		return value;
	}
}

uint64_t MiniIO_ReadLE64(mini_io_context *context) {
	uint64_t value = 0;
	MiniIO_Read(context, &value, sizeof(value), 1);
	return MINI_IO_BSWAP64_LE(value);
}

uint64_t MiniIO_ReadBE64(mini_io_context *context) {
	uint64_t value = 0;
	MiniIO_Read(context, &value, sizeof(value), 1);
	return MINI_IO_BSWAP64_BE(value);
}

uint64_t MiniIO_ReadU64(mini_io_context *context, int endian) {
	uint64_t value = 0;
	MiniIO_Read(context, &value, sizeof(value), 1);
	if (endian == MINI_IO_BIG_ENDIAN) {
		return MINI_IO_BSWAP64_BE(value);
	} else if (endian == MINI_IO_LITTLE_ENDIAN) {
		 return MINI_IO_BSWAP64_LE(value);
	} else {
		assert(0);
		return value;
	}
}

size_t MiniIO_WriteU8(mini_io_context *context, uint8_t value) {
	return MiniIO_Write(context, &value, sizeof(value), 1);
}

size_t MiniIO_WriteLE16(mini_io_context *context, uint16_t value) {
	const uint16_t swapped = MINI_IO_BSWAP16_LE(value);
	return MiniIO_Write(context, &swapped, sizeof(swapped), 1);
}

size_t MiniIO_WriteBE16(mini_io_context *context, uint16_t value) {
	const uint16_t swapped = MINI_IO_BSWAP16_BE(value);
	return MiniIO_Write(context, &swapped, sizeof(swapped), 1);
}

size_t MiniIO_WriteU16(mini_io_context *context, uint16_t value, int endian) {
	uint16_t swapped = value;
	if (endian == MINI_IO_BIG_ENDIAN) {
		swapped = MINI_IO_BSWAP16_BE(value);
	} else if (endian == MINI_IO_LITTLE_ENDIAN) {
		swapped = MINI_IO_BSWAP16_LE(value);
	} else {
		assert(0);
	}
	return MiniIO_Write(context, &swapped, sizeof(swapped), 1);
}

size_t MiniIO_WriteLE32(mini_io_context *context, uint32_t value) {
	const uint32_t swapped = MINI_IO_BSWAP32_LE(value);
	return MiniIO_Write(context, &swapped, sizeof(swapped), 1);
}

size_t MiniIO_WriteBE32(mini_io_context *context, uint32_t value) {
	const uint32_t swapped = MINI_IO_BSWAP32_BE(value);
	return MiniIO_Write(context, &swapped, sizeof(swapped), 1);
}

size_t MiniIO_WriteU32(mini_io_context *context, uint32_t value, int endian) {
	uint32_t swapped = value;
	if (endian == MINI_IO_BIG_ENDIAN) {
		swapped = MINI_IO_BSWAP32_BE(value);
	} else if (endian == MINI_IO_LITTLE_ENDIAN) {
		swapped = MINI_IO_BSWAP32_LE(value);
	} else {
		assert(0);
	}
	return MiniIO_Write(context, &swapped, sizeof(swapped), 1);
}

size_t MiniIO_WriteLE64(mini_io_context *context, uint64_t value) {
	const uint64_t swapped = MINI_IO_BSWAP64_LE(value);
	return MiniIO_Write(context, &swapped, sizeof(swapped), 1);
}

size_t MiniIO_WriteBE64(mini_io_context *context, uint64_t value) {
	const uint64_t swapped = MINI_IO_BSWAP64_BE(value);
	return MiniIO_Write(context, &swapped, sizeof(swapped), 1);
}

size_t MiniIO_WriteU64(mini_io_context *context, uint64_t value, int endian) {
	uint64_t swapped = value;
	if (endian == MINI_IO_BIG_ENDIAN) {
		swapped = MINI_IO_BSWAP64_BE(value);
	} else if (endian == MINI_IO_LITTLE_ENDIAN) {
		swapped = MINI_IO_BSWAP64_LE(value);
	} else {
		assert(0);
	}
	return MiniIO_Write(context, &swapped, sizeof(swapped), 1);
}

/***************************************/

#ifndef MINI_IO_DISABLE_STDIO
static const char *mini_io_mode_to_string(int mode) {
	switch (mode) {
	case MINI_IO_OPEN_READ: return "rb";
	case MINI_IO_OPEN_WRITE: return "wb";
	case MINI_IO_OPEN_APPEND: return "ab";
	case MINI_IO_OPEN_READ | MINI_IO_OPEN_UPDATE: return "r+b";
	case MINI_IO_OPEN_WRITE | MINI_IO_OPEN_UPDATE: return "w+b";
	case MINI_IO_OPEN_APPEND | MINI_IO_OPEN_UPDATE: return "a+b";
	default: return "";
	}
}
#endif

mini_io_context *MiniIO_OpenFile(const char *filename, int mode) {
#ifndef MINI_IO_DISABLE_STDIO
	const char *modestr = mini_io_mode_to_string(mode);
#if defined(_MSC_VER) && _MSC_VER >= 1400
	FILE *stream;
	if (fopen_s(&stream, filename, modestr) != 0) {
		return NULL;
	}
#else
	FILE *stream = fopen(filename, modestr);
#endif
	if (!stream) {
		return NULL;
	}
	return MiniIO_CreateFromFP(stream, 1);
#else
	MINI_IO_UNUSED(filename);
	MINI_IO_UNUSED(mode);
	return NULL;
#endif
}

mini_io_context *MiniIO_CreateContext(mini_io_callbacks *callbacks, void *data) {
	mini_io_context *context;
	context = malloc(sizeof(mini_io_context));
	if (!context) {
		return NULL;
	}

	context->callbacks = callbacks;
	context->data = data;
	context->autoclose = 0;
	context->readonly = 0;

	return context;
}

int MiniIO_DeleteContext(mini_io_context *context) {
	int retval = 0;
	if (context->autoclose) {
		retval = context->callbacks->close(context);
	}
	free(context->data);
	free(context);
	return retval;
}

#ifndef MINI_IO_DISABLE_STDIO

typedef struct {
	FILE *fp;
} mini_io_stdio_data;

static size_t mini_io_stdio_read(mini_io_context *context, void *ptr, size_t size, size_t maxnum) {
	mini_io_stdio_data *data = (mini_io_stdio_data *)context->data;
	return fread(ptr, size, maxnum, data->fp);
}

static size_t mini_io_stdio_write(mini_io_context *context, const void *ptr, size_t size, size_t num) {
	mini_io_stdio_data *data = (mini_io_stdio_data *)context->data;
	return fwrite(ptr, size, num, data->fp);
}

static void mini_io_stdio_seek(mini_io_context *context, off_t n, int whence) {
	mini_io_stdio_data *data = (mini_io_stdio_data *)context->data;
#if defined(_MSC_VER) || defined(__MINGW32__) || defined(__WATCOMC__)
	_fseeki64(data->fp, n, whence);
#else
	fseeko(data->fp, n, whence);
#endif
}

static off_t mini_io_stdio_tell(mini_io_context *context) {
	mini_io_stdio_data *data = (mini_io_stdio_data *)context->data;
#if defined(_MSC_VER) || defined(__MINGW32__) || defined(__WATCOMC__)
	return _ftelli64(data->fp);
#else
	return ftello(data->fp);
#endif
}

static off_t mini_io_stdio_size(mini_io_context *context) {
	off_t oldpos, length;

	oldpos = MiniIO_Tell(context);
	MiniIO_Seek(context, 0, MINI_IO_SEEK_END);
	length = MiniIO_Tell(context);
	MiniIO_Seek(context, oldpos, MINI_IO_SEEK_SET);

	return length;
}

static int mini_io_stdio_eof(mini_io_context *context) {
	mini_io_stdio_data *data = (mini_io_stdio_data *)context->data;
	return feof(data->fp);
}

static int mini_io_stdio_flush(mini_io_context *context) {
	mini_io_stdio_data *data = (mini_io_stdio_data *)context->data;
	return fflush(data->fp);
}

static int mini_io_stdio_close(mini_io_context *context) {
	mini_io_stdio_data *data = (mini_io_stdio_data *)context->data;
	return fclose(data->fp);
}

static mini_io_callbacks mini_io_stdio_callbacks = {
	mini_io_stdio_read,
	mini_io_stdio_write,
	mini_io_stdio_seek,
	mini_io_stdio_tell,
	mini_io_stdio_size,
	mini_io_stdio_eof,
	mini_io_stdio_flush,
	mini_io_stdio_close
};

mini_io_context *MiniIO_CreateFromFP(FILE *stream, int autoclose) {
	mini_io_context *context;
	mini_io_stdio_data *data = malloc(sizeof(mini_io_stdio_data));
	if (!data) {
		return NULL;
	}

	data->fp = stream;

	context = MiniIO_CreateContext(&mini_io_stdio_callbacks, data);
	if (!context) {
		free(data);
		return NULL;
	}

	context->autoclose = autoclose;
	return context;
}

#endif

typedef struct {
	char *mem, *end, *ptr;
} mini_io_mem_data;

static size_t mini_io_mem_read(mini_io_context *context, void *ptr, size_t size, size_t maxnum) {
	mini_io_mem_data *data = (mini_io_mem_data *)context->data;
	size_t total;

	if (size == 0 || maxnum == 0)
		return 0;

	total = (data->end - data->ptr) / size;
	if (total == 0)
		return 0;
	else if (total < maxnum)
		maxnum = total;

	memcpy(ptr, data->ptr, maxnum * size);
	data->ptr += maxnum * size;

	return maxnum;
}

static size_t mini_io_mem_write(mini_io_context *context, const void *ptr, size_t size, size_t num) {
	mini_io_mem_data *data = (mini_io_mem_data *)context->data;
	if (context->readonly)
		return 0;

	if ((data->ptr + (num * size)) > data->end) {
		num = (data->end - data->ptr) / size;
	}

	memcpy(data->ptr, ptr, num * size);
	data->ptr += num * size;

	return num;
}

static void mini_io_mem_seek(mini_io_context *context, off_t n, int whence) {
	mini_io_mem_data *data = (mini_io_mem_data *)context->data;
	char *newpos;

	switch(whence) {
	case MINI_IO_SEEK_SET: newpos = data->mem + n; break;
	case MINI_IO_SEEK_CUR: newpos = data->ptr + n; break;
	case MINI_IO_SEEK_END: newpos = data->end + n; break;
	default:
		assert(0);
		return;
	}

	data->ptr = MINI_IO_CLAMP(newpos, data->mem, data->end);
}

static off_t mini_io_mem_tell(mini_io_context *context) {
	mini_io_mem_data *data = (mini_io_mem_data *)context->data;
	return data->ptr - data->mem;
}

static off_t mini_io_mem_size(mini_io_context *context) {
	mini_io_mem_data *data = (mini_io_mem_data *)context->data;
	return data->end - data->mem;
}

static int mini_io_mem_eof(mini_io_context *context) {
	mini_io_mem_data *data = (mini_io_mem_data *)context->data;
	return data->ptr == data->end;
}

static int mini_io_mem_flush(mini_io_context *context) {
	MINI_IO_UNUSED(context);
	return 0;
}

static int mini_io_mem_close(mini_io_context *context) {
	MINI_IO_UNUSED(context);
	return 0;
}

static mini_io_callbacks mini_io_mem_callbacks = {
	mini_io_mem_read,
	mini_io_mem_write,
	mini_io_mem_seek,
	mini_io_mem_tell,
	mini_io_mem_size,
	mini_io_mem_eof,
	mini_io_mem_flush,
	mini_io_mem_close
};

mini_io_context *MiniIO_CreateFromMem(void *mem, size_t size) {
	mini_io_context *context;
	mini_io_mem_data *data = malloc(sizeof(mini_io_mem_data));
	if (!data) {
		return NULL;
	}

	data->mem = mem;
	data->end = data->mem + size;
	data->ptr = data->mem;

	context = MiniIO_CreateContext(&mini_io_mem_callbacks, data);
	if (!context) {
		free(data);
		return NULL;
	}

	return context;
}

mini_io_context *MiniIO_CreateFromConstMem(const void *mem, size_t size) {
	mini_io_context *context = MiniIO_CreateFromMem((void *)mem, size);
	if (!context) {
		return NULL;
	}

	context->readonly = 1;
	return context;
}

typedef struct {
	mini_io_context *parent;
	off_t start, pos, end;
	int safe;
} mini_io_sub_data;

static size_t mini_io_sub_read(mini_io_context *context, void *ptr, size_t size, size_t maxnum) {
	mini_io_sub_data *data = (mini_io_sub_data *)context->data;
	size_t i, read = 0;

	if (data->safe) {
		MiniIO_Seek(context, 0, MINI_IO_SEEK_CUR);
	}

	for (i = 0; i < maxnum; i++) {
		if (data->pos >= data->end)
			break;
		if (MiniIO_Read(data->parent, ptr, size, 1) == 0)
			break;
		ptr = (char *)ptr + size;
		data->pos += size;
		read++;
	}

	return read;
}

static size_t mini_io_sub_write(mini_io_context *context, const void *ptr, size_t size, size_t num) {
	MINI_IO_UNUSED(context);
	MINI_IO_UNUSED(ptr);
	MINI_IO_UNUSED(size);
	MINI_IO_UNUSED(num);
	return 0;
}

static void mini_io_sub_seek(mini_io_context *context, off_t n, int whence) {
	mini_io_sub_data *data = (mini_io_sub_data *)context->data;
	off_t newpos;

	switch(whence) {
	case MINI_IO_SEEK_SET: newpos = data->start + n; break;
	case MINI_IO_SEEK_CUR: newpos = data->pos + n; break;
	case MINI_IO_SEEK_END: newpos = data->end + n; break;
	default:
		assert(0);
		return;
	}

	newpos = MINI_IO_CLAMP(newpos, data->start, data->end);
	MiniIO_Seek(data->parent, newpos, MINI_IO_SEEK_SET);
}

static off_t mini_io_sub_tell(mini_io_context *context) {
	mini_io_sub_data *data = (mini_io_sub_data *)context->data;
	return data->pos - data->start;
}

static off_t mini_io_sub_size(mini_io_context *context) {
	mini_io_sub_data *data = (mini_io_sub_data *)context->data;
	return data->end - data->start;
}

static int mini_io_sub_eof(mini_io_context *context) {
	mini_io_sub_data *data = (mini_io_sub_data *)context->data;
	return (data->pos == data->end) | MiniIO_EOF(data->parent);
}

static int mini_io_sub_flush(mini_io_context *context) {
	mini_io_sub_data *data = (mini_io_sub_data *)context->data;
	return MiniIO_Flush(data->parent);
}

static int mini_io_sub_close(mini_io_context *context) {
	mini_io_sub_data *data = (mini_io_sub_data *)context->data;
	return MiniIO_DeleteContext(data->parent);
}

static mini_io_callbacks mini_io_sub_callbacks = {
	mini_io_sub_read,
	mini_io_sub_write,
	mini_io_sub_seek,
	mini_io_sub_tell,
	mini_io_sub_size,
	mini_io_sub_eof,
	mini_io_sub_flush,
	mini_io_sub_close
};

mini_io_context *MiniIO_CreateFromContext(mini_io_context *parent, off_t start, off_t size, int autoclose, int safe) {
	mini_io_context *context;
	mini_io_sub_data *data = malloc(sizeof(mini_io_sub_data));
	if (!data) {
		return NULL;
	}

	data->parent = parent;
	data->start = start;
	data->pos = start;
	data->end = start + size;
	data->safe = safe;

	context = MiniIO_CreateContext(&mini_io_sub_callbacks, data);
	if (!context) {
		free(data);
		return NULL;
	}

	MiniIO_Seek(data->parent, data->start, MINI_IO_SEEK_SET);
	context->autoclose = autoclose;
	return context;
}

#endif
