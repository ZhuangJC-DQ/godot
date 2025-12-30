/**************************************************************************/
/*  world.cpp                                                             */
/**************************************************************************/

#include "world.h"

#include "core/string/print_string.h"
#include "core/variant/variant.h"

World::World() {
}

World::~World() {
	clear();
}

Chunk *World::get_chunk(int32_t x, int32_t y) {
	ChunkCoord coord(x, y);
	uint64_t key = coord.to_seed();

	if (chunks.has(key)) {
		return chunks[key];
	}

	Chunk *chunk = new Chunk(coord);
	chunks[key] = chunk;
	return chunk;
}

void World::print_chunk(int32_t x, int32_t y, int preview_size) {
	Chunk *chunk = get_chunk(x, y);
	print_line(chunk->to_string(preview_size));
}

void World::clear() {
	for (KeyValue<uint64_t, Chunk *> &kv : chunks) {
		delete kv.value;
	}
	chunks.clear();
}
