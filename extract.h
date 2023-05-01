#ifndef EXTRACT_H
#define EXTRACT_H

#include "archive.h"

#include <stdio.h>

FILE *open_output_file(const char *path, const char *name, int ftype, int nfs_exts);

uint32_t dump_entry_to_file(FILE *context, biik_archive_entry *entry, const char *path, int nfs_exts);
uint32_t dump_tracker_to_file(FILE *context, biik_archive_entry *entry, const char *path, int nfs_exts);
uint32_t dump_script_to_file(FILE *context, biik_archive_entry *entry, const char *path, int nfs_exts);
uint32_t dump_to_file(FILE *context, biik_archive_entry *entry, const char *path, int nfs_exts, int convert);

#endif
