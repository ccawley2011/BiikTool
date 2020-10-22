#include <string.h>
#include <assert.h>

#include "archive.h"
#include "debug.h"
#include "mini_io.h"

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

mini_io_context *open_archive_entry(mini_io_context *input, biik_archive_entry *entry, int safe) {
	uint32_t start = entry->offset + entry->header_size;
	uint32_t size = entry->size - entry->header_size;
	return MiniIO_CreateFromContext(input, start, size, 0, safe);
}

biik_archive_entry *read_archive_entry(mini_io_context *archive, int endian) {
	biik_archive_entry *entry;
	off_t last_pos, file_size;

	entry = malloc(sizeof(biik_archive_entry));
	if (!entry) {
		warning("Out of memory");
		return NULL;
	}
	memset(entry, 0, sizeof(biik_archive_entry));

	entry->entry_size = MiniIO_ReadU32(archive, endian);
	entry->offset = MiniIO_ReadU32(archive, endian);

	entry->name = malloc(entry->entry_size - 0x8);
	if (!entry->name) {
		warning("Out of memory");
		free(entry);
		return NULL;
	}
	MiniIO_Read(archive, entry->name, 1, entry->entry_size - 0x8);

	last_pos = MiniIO_Tell(archive);
	file_size = MiniIO_Size(archive);
	MiniIO_Seek(archive, entry->offset, MINI_IO_SEEK_SET);

	entry->type = MiniIO_ReadU32(archive, endian);
	entry->size = MiniIO_ReadU32(archive, endian);
	entry->header_size = MiniIO_ReadU32(archive, endian);
	if ((entry->offset + entry->size > (uint32_t)file_size) ||
	    (entry->offset + entry->header_size > (uint32_t)file_size) ||
	    (entry->header_size > entry->size)) {
		warningf("Invalid header for file '%s'", entry->name);
		MiniIO_Seek(archive, last_pos, MINI_IO_SEEK_SET);
		free(entry->name);
		free(entry);
		return NULL;
	}

	entry->name2 = malloc(entry->header_size - 0xC);
	if (!entry->name2) {
		warning("Out of memory");
		MiniIO_Seek(archive, last_pos, MINI_IO_SEEK_SET);
		free(entry->name);
		free(entry);
		return NULL;
	}
	MiniIO_Read(archive, entry->name2, 1, entry->header_size - 0xC);

	if (strcmp(entry->name, entry->name2) != 0)
		warningf("Entry names '%s' and '%s' do not match", entry->name, entry->name2);

	MiniIO_Seek(archive, last_pos, MINI_IO_SEEK_SET);
	return entry;
}

void free_archive_entry(biik_archive_entry *entry) {
	if (!entry)
		return;

	free(entry->name);
	free(entry->name2);
	free(entry);
}

biik_archive_header *read_archive_header(mini_io_context *archive) {
	biik_archive_header *header;
	int endian = MINI_IO_LITTLE_ENDIAN;
	off_t file_size = MiniIO_Size(archive);
	if (file_size < 18) {
		warning("This is not a Biik archive");
		return NULL;
	}

	header = malloc(sizeof(biik_archive_header));
	if (!header) {
		warning("Out of memory");
		return NULL;
	}
	memset(header, 0, sizeof(biik_archive_header));

	MiniIO_Read(archive, header->signature, 1, 8);
	header->size = MiniIO_ReadLE32(archive);
	if (header->size > 0xFFFFFF) {
		header->size = MINI_IO_BSWAP32(header->size);
		endian = MINI_IO_BIG_ENDIAN;
	}

	if (strncmp(header->signature, "BIIK-DJC", 8) != 0) {
		warning("This is not a Biik archive");
		free(header);
		return NULL;
	}
	if (header->size < 0x18) {
		warning("File is too small to be a Biik archive");
		free(header);
		return NULL;
	}

	header->version = MiniIO_ReadU32(archive, endian);
	header->game_id = MiniIO_ReadU32(archive, endian);
	header->index_offset = MiniIO_ReadU32(archive, endian);

	if ((uint32_t)file_size < header->index_offset + 12) {
		warning("Archive index is outside the file");
		free(header);
		return NULL;
	}

	MiniIO_Seek(archive, header->index_offset, MINI_IO_SEEK_SET);

	header->unknown = MiniIO_ReadU32(archive, endian);
	header->index_size = MiniIO_ReadU32(archive, endian);
	header->index_header_size = MiniIO_ReadU32(archive, endian);
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
	MiniIO_Read(archive, header->index_title, 1, header->index_header_size - 0xC);

	if (strcmp(header->index_title, "File index") != 0 || header->unknown != 3)
		warningf("Unexpected index header: '%s' (%d)", header->index_title, header->unknown);

	header->entry_count = 0;
	header->entries = malloc(sizeof(void *));
	while (MiniIO_Tell(archive) < file_size) {
		void *new_block;
		header->entries[header->entry_count++] =
			read_archive_entry(archive, endian);

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
