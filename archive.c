#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "archive.h"
#include "utils.h"

const char *get_game_name(uint32_t id) {
	switch (id) {
	case GAME_ID_DINODEMO:  return "Dinosaur Discovery demo";
	case GAME_ID_MOUSE:     return "A Mouse in Holland";
	case GAME_ID_FLOSSY:    return "Explore with Flossy the Frog";
	case GAME_ID_DINOSAUR:  return "Dinosaur Discovery";
	case GAME_ID_BETSI:     return "Betsi the Tudor Dog";
	case GAME_ID_GUARDIANS: return "Guardians of the Greenwood";
	case GAME_ID_PATCH:     return "Patch the Puppy";
	case GAME_ID_FIFI:      return "Find It Fix It";
	case GAME_ID_DARRYL:    return "Darryl the Dragon";
	default:                return "(unknown game)";
	}
}

const char *get_type_name(uint32_t type) {
	switch (type) {
	case ENTRY_TYPE_GRAPHIC: return "Graphic";
	case ENTRY_TYPE_TRACKER: return "Tracker";
	case ENTRY_TYPE_SCRIPT:  return "Script";
	case ENTRY_TYPE_DRAW:    return "Draw";
	case ENTRY_TYPE_COORDS:  return "Co-ordinates";
	default:                 return "(unknown)";
	}
}

int entry_to_file_type(uint32_t type) {
	switch (type) {
	case ENTRY_TYPE_TRACKER: return 0xCB6;
	case ENTRY_TYPE_DRAW:    return 0xAFF;
	default:                 return 0xFFD;
	}
}

biik_archive_entry *read_archive_entry(FILE *archive, long file_size, int be) {
	biik_archive_entry *entry;
	long last_pos;
	unsigned char buf[12];

	entry = malloc(sizeof(biik_archive_entry));
	if (!entry) {
		warning("Out of memory");
		return NULL;
	}
	memset(entry, 0, sizeof(biik_archive_entry));

	fread(buf, 8, 1, archive);
	entry->entry_size = read_u32(buf + 0, be);
	entry->offset = read_u32(buf + 4, be);

	entry->name = malloc(entry->entry_size - 0x8);
	if (!entry->name) {
		warning("Out of memory");
		free(entry);
		return NULL;
	}
	fread(entry->name, 1, entry->entry_size - 0x8, archive);

	last_pos = ftell(archive);
	fseek(archive, entry->offset, SEEK_SET);

	fread(buf, 12, 1, archive);
	entry->type = read_u32(buf + 0, be);
	entry->size = read_u32(buf + 4, be);
	entry->header_size = read_u32(buf + 8, be);
	if ((entry->offset + entry->size > (uint32_t)file_size) ||
	    (entry->offset + entry->header_size > (uint32_t)file_size) ||
	    (entry->header_size > entry->size)) {
		warningf("Invalid header for file '%s'", entry->name);
		fseek(archive, last_pos, SEEK_SET);
		free(entry->name);
		free(entry);
		return NULL;
	}

	entry->name2 = malloc(entry->header_size - 0xC);
	if (!entry->name2) {
		warning("Out of memory");
		fseek(archive, last_pos, SEEK_SET);
		free(entry->name);
		free(entry);
		return NULL;
	}
	fread(entry->name2, 1, entry->header_size - 0xC, archive);

	if (strcmp(entry->name, entry->name2) != 0)
		warningf("Entry names '%s' and '%s' do not match", entry->name, entry->name2);

	fseek(archive, last_pos, SEEK_SET);
	return entry;
}

void free_archive_entry(biik_archive_entry *entry) {
	if (!entry)
		return;

	free(entry->name);
	free(entry->name2);
	free(entry);
}

biik_archive_header *read_archive_header(FILE *archive) {
	biik_archive_header *header;
	int be = 0;
	long file_size;
	unsigned char buf[24];

	file_size = fsize(archive);

	if (fread(buf, 24, 1, archive) != 1) {
		warning("Could not read archive header");
		return NULL;
	}

	if (memcmp(buf, "BIIK-DJC", 8) != 0) {
		warning("This is not a Biik archive");
		return NULL;
	}

	if (buf[11]) {
		be = 1;
	}

	header = malloc(sizeof(biik_archive_header));
	if (!header) {
		warning("Out of memory");
		return NULL;
	}
	memset(header, 0, sizeof(biik_archive_header));

	memcpy(header->signature, buf, 8);
	header->size = read_u32(buf + 8, be);
	header->version = read_u32(buf + 12, be);
	header->game_id = read_u32(buf + 16, be);
	header->index_offset = read_u32(buf + 20, be);

	if (header->size < 24) {
		warning("File is too small to be a Biik archive");
		free(header);
		return NULL;
	}

	if ((uint32_t)file_size < header->index_offset + 12) {
		warning("Archive index is outside the file");
		free(header);
		return NULL;
	}

	fseek(archive, header->index_offset, SEEK_SET);

	fread(buf, 12, 1, archive);
	header->unknown = read_u32(buf + 0, be);
	header->index_size = read_u32(buf + 4, be);
	header->index_header_size = read_u32(buf + 8, be);
	if ((uint32_t)file_size < header->index_offset + header->index_header_size) {
		warning("Invalid header for archive index");
		free(header);
		return NULL;
	}

	if ((uint32_t)file_size < header->index_offset + header->index_size)
		warning("Archive index may have been truncated");

	header->index_title = malloc(header->index_header_size - 0xC);
	if (!header->index_title) {
		warning("Out of memory");
		free(header);
		return NULL;
	}
	fread(header->index_title, 1, header->index_header_size - 0xC, archive);

	if (strcmp(header->index_title, "File index") != 0 || header->unknown != 3)
		warningf("Unexpected index header: '%s' (%d)", header->index_title, header->unknown);

	header->entry_count = 0;
	header->entries = malloc(sizeof(void *));
	while (ftell(archive) < file_size) {
		void *new_block;
		header->entries[header->entry_count++] =
			read_archive_entry(archive, file_size, be);

		new_block = realloc(header->entries,
			(header->entry_count + 1) * sizeof(void *));
		if (new_block) {
			header->entries = new_block;
		} else {
			warning("Out of memory");
			break;
		}
	}

	return header;
}

void free_archive_header(biik_archive_header *header) {
	int i;

	if (!header)
		return;

	for (i = 0; i < header->entry_count; i++)
		free_archive_entry(header->entries[i]);

	free(header->entries);
	free(header->index_title);
	free(header);
}
