#include "archive.h"
#include "compress.h"
#include "extract.h"
#include "musx.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>

#ifdef __riscos
#include "kernel.h"
#include "swis.h"
#elif defined(_WIN32) || defined(__WATCOMC__)
#include <direct.h>
#include <io.h>
#else
#include <sys/stat.h>
#endif

#if defined(_MSC_VER) && (_MSC_VER < 1900)
#define snprintf(a,b,...) _snprintf_s(a,b,_TRUNCATE,__VA_ARGS__)
#endif

#ifdef __riscos
# define PATH_SEP "."
#elif defined(_WIN32)
# define PATH_SEP "\\"
#else
# define PATH_SEP "/"
#endif

static void my_mkdir(const char *path) {
#ifdef __riscos
	_kernel_swi_regs regs;
	regs.r[0] = 8;
	regs.r[1] = (int)path;
	_kernel_swi(OS_File, &regs, &regs);
#elif defined(_WIN32)
	_mkdir(path);
#elif defined(__WATCOMC__)
	mkdir(path);
#else
	mkdir(path, 0755);
#endif
}

static void my_settype(const char *path, int type) {
#ifdef __riscos
	_kernel_swi_regs regs;
	regs.r[0] = 18;
	regs.r[1] = (int)path;
	regs.r[2] = type;
	_kernel_swi(OS_File, &regs, &regs);
#else
	(void)path;
	(void)type;
#endif
}

FILE *open_output_file(const char *path, const char *name, int ftype, int nfs_exts) {
	FILE *output;
	char filename[1024];

	if (nfs_exts) {
		snprintf(filename, 1024, "%s" PATH_SEP "%s,%03x", path, name, ftype);
	} else {
		snprintf(filename, 1024, "%s" PATH_SEP "%s", path, name);
	}

	my_mkdir(path);
	output = fopen(filename, "wb");
	if (!output) {
		warningf("Could not open file %s", filename);
		return NULL;
	}

	my_settype(filename, ftype);

	return output;
}

uint32_t dump_entry_to_file(FILE *input, biik_archive_entry *entry, const char *path, int nfs_exts) {
	FILE *output;
	size_t size, read;

	output = open_output_file(path, entry->name, entry_to_file_type(entry->type), nfs_exts);
	if (!output) {
		return 0;
	}

	size = (size_t)(entry->size - entry->header_size);
	read = fcopy(input, output, size);

	fclose(output);

	return (uint32_t)(read);
}

uint32_t dump_tracker_to_file(FILE *input, biik_archive_entry *entry, const char *path, int nfs_exts) {
	FILE *output;
	uint32_t size;

	output = open_output_file(path, entry->name, 0xcb6, nfs_exts);
	if (!output) {
		return 0;
	}

	size = decompress_musx(input, output);

	fclose(output);

	return size;
}

uint32_t dump_script_to_file(FILE *input, biik_archive_entry *entry, const char *path, int nfs_exts) {
	FILE *output;
	uint32_t unknown, size, realsize;
	unsigned char buf[8];
	char *block;

	fread(buf, 8, 1, input);

	if (buf[3]) {
		unknown = read_u32(buf + 0, 1);
		size = read_u32(buf + 4, 1);
	} else {
		unknown = read_u32(buf + 0, 0);
		size = read_u32(buf + 4, 0);
	}
	if (unknown != 1)
		warningf("Unexpected script header: %d", unknown);

	block = malloc(size);
	if (!block) {
		return 0;
	}

	realsize = lzw_decode(input, block, size);
	if (size != realsize)
		warningf("Unexpected end of stream in file %s: expected %d bytes, got %d bytes", entry->name, size, realsize);

	output = open_output_file(path, entry->name, 0xfff, nfs_exts);
	if (!output) {
		free(block);
		return 0;
	}

	fwrite(block, realsize, 1, output);
	fclose(output);
	free(block);

	return realsize;
}

uint32_t dump_to_file(FILE *context, biik_archive_entry *entry, const char *path, int nfs_exts, int convert) {
	fseek(context, (long)(entry->offset + entry->header_size), SEEK_SET);

	if (convert) {
		switch (entry->type) {
		case ENTRY_TYPE_TRACKER:
			return dump_tracker_to_file(context, entry, path, nfs_exts);
		case ENTRY_TYPE_SCRIPT:
			return dump_script_to_file(context, entry, path, nfs_exts);
		}
	}
	return dump_entry_to_file(context, entry, path, nfs_exts);
}
