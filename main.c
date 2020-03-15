#include "archive.h"
#include "debug.h"
#include "extract.h"

#if defined(__TARGET_SCL__) || defined(_MSC_VER) || defined(__WATCOMC__)
#include "ext/getopt/getopt.h"
#else
#include <getopt.h>
#endif

#define MINI_IO_IMPLEMENTATION
#include "mini_io.h"

const char *syntax_string = "Syntax: %s [-c] [-l] [-n] [-o <output dir>] [-q] <filename>\n";

int main(int argc, char **argv) {
	mini_io_context *context;
	biik_archive_header *header;
	int convert = 0, list_files = 0, nfs_ext = 0, quiet = 0;
	const char *infile = NULL, *outpath = NULL;
	int i, c;

	while ((c = getopt(argc, argv, "clno:q")) != -1) {
		switch (c) {
		case 'c':
			convert = 1;
			break;
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
			dump_to_file(context, entry, outpath, nfs_ext, convert);
		}
	}
	free_archive_header(header);
	if (list_files)
		fputc('\n', stderr);

	MiniIO_DeleteContext(context);
	return 0;
}
