/**************************************************************************/
/*  world_manager.cpp                                                     */
/**************************************************************************/

#include "world_manager.h"

#include "chunk.h"
#include "core/object/class_db.h"

WorldManager::WorldManager() {
}

WorldManager::~WorldManager() {
}

void WorldManager::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_chunk_data", "chunk_x", "chunk_y"), &WorldManager::get_chunk_data);
	ClassDB::bind_method(D_METHOD("get_tile_type", "chunk_x", "chunk_y", "tile_x", "tile_y"), &WorldManager::get_tile_type);
}

Dictionary WorldManager::get_chunk_data(int32_t chunk_x, int32_t chunk_y) {
	Dictionary data;
	Chunk *chunk = world.get_chunk(chunk_x, chunk_y);
	if (!chunk) {
		return data;
	}

	data["coord_x"] = chunk_x;
	data["coord_y"] = chunk_y;

	return data;
}

int WorldManager::get_tile_type(int32_t chunk_x, int32_t chunk_y, int32_t tile_x, int32_t tile_y) {
	Chunk *chunk = world.get_chunk(chunk_x, chunk_y);
	if (!chunk || tile_x < 0 || tile_x >= CHUNK_SIZE || tile_y < 0 || tile_y >= CHUNK_SIZE) {
		return -1;
	}

	return static_cast<int>(chunk->tiles[tile_y][tile_x]);
}
