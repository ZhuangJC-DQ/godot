/**************************************************************************/
/*  world.h                                                               */
/**************************************************************************/

#pragma once

#include "chunk.h"
#include "core/templates/hash_map.h"

class World {
private:
	HashMap<uint64_t, Chunk *> chunks;

public:
	World();
	~World();

	Chunk *get_chunk(int32_t x, int32_t y);
	void print_chunk(int32_t x, int32_t y, int preview_size = 32);
	void clear();
};
