#ifndef ARCHIVE_H
#define ARCHIVE_H

#include "mini_io.h"

#include <stdbool.h>
#include <stdint.h>

typedef struct biik_archive_entry {
	uint32_t entry_size;
	uint32_t offset;
	char    *name;

	uint32_t type;
	uint32_t size;
	uint32_t header_size;
	char    *name2;
} biik_archive_entry;

enum {
	ENTRY_TYPE_GRAPHIC = 1,
	ENTRY_TYPE_TRACKER = 2,
	ENTRY_TYPE_SCRIPT = 9,
	ENTRY_TYPE_DRAW = 11,
	ENTRY_TYPE_COORDS = 15
};

typedef struct biik_archive_header {
	char     signature[8];
	uint32_t size;
	uint32_t version;
	uint32_t game_id;
	uint32_t index_offset;

	uint32_t unknown;
	uint32_t index_size;
	uint32_t index_header_size;
	char    *index_title;

	int entry_count;
	biik_archive_entry **entries;
} biik_archive_header;

enum {
	GAME_ID_DINOSAUR = 1,
	GAME_ID_MOUSE = 3,
	GAME_ID_FLOSSY = 5,
	GAME_ID_BETSI = 8,
	GAME_ID_GUARDIANS = 13,
	GAME_ID_FIFI = 18,
	GAME_ID_DARRYL = 255
};

const char *get_game_name(uint32_t id);
const char *get_type_name(uint32_t id);

int entry_to_file_type(uint32_t type, bool converted);

biik_archive_header *read_archive_header(mini_io_context *archive);
void free_archive_header(biik_archive_header *header);

#endif
