#include <stdarg.h>

#ifdef __riscos__
#include <kernel.h>
#include <swis.h>
#elif defined(_WIN32)
#include <io.h>
#endif

#include "archive.h"
#include "ext/getopt/getopt.h"

#define MINI_IO_IMPLEMENTATION
#include "mini_io.h"

void warning(const char *message) {
	fputs("WARNING: ", stderr);
	fputs(message, stderr);
	fputc('\n', stderr);
}

void warningf(const char *message, ...) {
	va_list args;

	fputs("WARNING: ", stderr);

	va_start(args, message);
	vfprintf(stderr, message, args);
	va_end(args);

	fputc('\n', stderr);
}

#ifdef __riscos__
# define PATH_SEP "."
#elif defined(_WIN32)
# define PATH_SEP "\\"
#else
# define PATH_SEP "/"
#endif

void my_mkdir(const char *path) {
#ifdef __riscos__
	_kernel_swi_regs regs;
	regs.r[0] = 8;
	regs.r[1] = (int)path;
	_kernel_swi(OS_File, &regs, &regs);
#elif defined(_WIN32)
	_mkdir(path);
#else
	mkdir(path, 0755);
#endif
}

void my_settype(const char *path, int type) {
#ifdef __riscos__
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

bool dump_entry_to_file(mini_io_context *input, biik_archive_entry *entry, const char *path, bool nfs_exts) {
	uint32_t offset = entry->offset + entry->header_size;
	uint32_t size = entry->size - entry->header_size;
	mini_io_context *output;
	char filename[1024];
	char *block = malloc(size);
	if (!block)
		return false;

	MiniIO_Seek(input, offset, MINI_IO_SEEK_SET);
	MiniIO_Read(input, block, 1, size);

	if (nfs_exts) {
		snprintf(filename, 1024, "%s" PATH_SEP "%s,%03x", path,
			entry->name, entry_to_file_type(entry->type, false));
	} else {
		snprintf(filename, 1024, "%s" PATH_SEP "%s", path, entry->name);
	}

	output = MiniIO_OpenFile(filename, MINI_IO_OPEN_WRITE);
	if (!output) {
		warningf("Could not open file %s", filename);
		free(block);
		return false;
	}

	MiniIO_Write(output, block, 1, size);
	MiniIO_DeleteContext(output);

	my_settype(filename, entry_to_file_type(entry->type, false));

	free(block);

	return true;
}

const char *syntax_string = "Syntax: %s [-n] [-o <output dir>] [-q] <filename>\n";

int main(int argc, char **argv) {
	mini_io_context *context;
	biik_archive_header *header;
	bool nfs_ext = false, quiet = false;
	const char *infile = NULL, *outpath = NULL;
	int i, c;

	while ((c = getopt(argc, argv, "no:q")) != -1) {
		switch (c) {
		case 'n':
			nfs_ext = true;
			break;
		case 'o':
			outpath = optarg;
			break;
		case 'q':
			quiet = true;
			break;
		case '?':
			return 1;
		default:
			abort();
		}
	}

	if (argc - optind < 1) {
		fprintf(stderr, syntax_string, argv[0]);
		return 1;
	} else {
		infile = argv[optind];
	}

	context = MiniIO_OpenFile(infile, MINI_IO_OPEN_READ);
	if (!context) {
		warningf("Could not open file %s", infile);
		return 1;
	}

	header = read_archive_header(context);
	if (!header) {
		MiniIO_DeleteContext(context);
		return 1;
	}

	if (!quiet)
		fprintf(stderr, "Opening %s archive (version %d)\n",
			get_game_name(header->game_id), header->version);

	for (i = 0; i < header->entry_count; i++) {
		biik_archive_entry *entry = header->entries[i];

		if (!quiet)
			fprintf(stderr, "  %s    %s (%d)\n", entry->name,
				get_type_name(entry->type), entry->type);

		if (outpath) {
			my_mkdir(outpath);
			dump_entry_to_file(context, entry, outpath, nfs_ext);
		}
	}
	free_archive_header(header);
	if (!quiet)
		fputc('\n', stderr);

	MiniIO_DeleteContext(context);
	return 0;
}
