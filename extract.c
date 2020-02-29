#include "archive.h"
#include "debug.h"
#include "extract.h"
#include "mini_io.h"

#ifdef __riscos__
#include <kernel.h>
#include <swis.h>
#elif defined(_WIN32) || defined(__WATCOMC__)
#include <direct.h>
#include <io.h>
#else
#include <sys/stat.h>
#endif

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

static void my_mkdir(const char *path) {
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

static void my_settype(const char *path, int type) {
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

	my_mkdir(path);
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
	mini_io_context *output = open_output_file(path, entry->name, entry_to_file_type(entry->type), nfs_exts);
	if (!input || !output)
		return;

	MiniIO_Copy(input, output, (size_t)MiniIO_Size(input), 1);

	MiniIO_DeleteContext(output);
	MiniIO_DeleteContext(input);
}
