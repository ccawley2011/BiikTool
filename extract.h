#ifndef EXTRACT_H
#define EXTRACT_H

#include "archive.h"
#include "mini_io.h"

mini_io_context *open_output_file(const char *path, const char *name, int ftype, int nfs_exts);

uint32_t dump_entry_to_file(mini_io_context *context, biik_archive_entry *entry, const char *path, int nfs_exts);
uint32_t dump_script_to_file(mini_io_context *context, biik_archive_entry *entry, const char *path, int nfs_exts);
uint32_t dump_to_file(mini_io_context *context, biik_archive_entry *entry, const char *path, int nfs_exts, int convert);

#endif
