#include <stdarg.h>
#include "archive.h"
#include "debug.h"

#ifdef __riscos__
#include <kernel.h>
#include <swis.h>
#elif defined(_WIN32) || defined(__WATCOMC__)
#include <direct.h>
#include <io.h>
#else
#include <sys/stat.h>
#endif

#if defined(__TARGET_SCL__) || defined(_MSC_VER) || defined(__WATCOMC__)
#include "ext/getopt/getopt.h"
#else
#include <getopt.h>
#endif

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

#if defined(_MSC_VER) && (_MSC_VER < 1900)
#define snprintf(a,b,...) _snprintf_s(a,b,_TRUNCATE,__VA_ARGS__)
#endif

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
#elif defined(__WATCOMC__)
	mkdir(path);
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

mini_io_context *open_output_file(const char *path, const char *name, int ftype, int nfs_exts) {
	mini_io_context *output;
	char filename[1024];

	if (nfs_exts) {
		snprintf(filename, 1024, "%s" PATH_SEP "%s,%03x", path, name, ftype);
	} else {
		snprintf(filename, 1024, "%s" PATH_SEP "%s", path, name);
	}

	output = MiniIO_OpenFile(filename, MINI_IO_OPEN_WRITE);
	if (!output) {
		warningf("Could not open file %s", filename);
		return NULL;
	}

	my_settype(filename, ftype);

	return output;
}

void dump_entry_to_file(mini_io_context *context, biik_archive_entry *entry, const char *path, int nfs_exts) {
	mini_io_context *input = open_archive_entry(context, entry, 0);
	mini_io_context *output = open_output_file(path, entry->name, entry_to_file_type(entry->type, 0), nfs_exts);
	if (!input || !output)
		return;

	MiniIO_Copy(input, output, (size_t)MiniIO_Size(input), 1);

	MiniIO_DeleteContext(output);
	MiniIO_DeleteContext(input);
}

const char *syntax_string = "Syntax: %s [-l] [-n] [-o <output dir>] [-q] <filename>\n";

int main(int argc, char **argv) {
	mini_io_context *context;
	biik_archive_header *header;
	int list_files = 0, nfs_ext = 0, quiet = 0;
	const char *infile = NULL, *outpath = NULL;
	int i, c;

	while ((c = getopt(argc, argv, "lno:q")) != -1) {
		switch (c) {
		case 'l':
			list_files = 1;
			break;
		case 'n':
			nfs_ext = 1;
			break;
		case 'o':
			outpath = optarg;
			break;
		case 'q':
			quiet = 1;
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
		if (!entry)
			continue;

		if (list_files)
			fprintf(stderr, "  %s    %s (%d)\n", entry->name,
				get_type_name(entry->type), entry->type);

		if (outpath) {
			my_mkdir(outpath);
			dump_entry_to_file(context, entry, outpath, nfs_ext);
		}
	}
	free_archive_header(header);
	if (list_files)
		fputc('\n', stderr);

	MiniIO_DeleteContext(context);
	return 0;
}
