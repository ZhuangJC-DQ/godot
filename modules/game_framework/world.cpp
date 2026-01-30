/**************************************************************************/
/*  world.cpp                                                             */
/**************************************************************************/

#include "world.h"

#include "core/string/print_string.h"
#include "core/variant/variant.h"

World::World() :
		seed(1337) {
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

	// 1. 生成地形
	Chunk *chunk = new Chunk(coord, seed, noise_config);

	// 2. 生成城镇（Chunk 内泊松圆盘采样）
	chunk->towns = TownGenerator::generate_towns_in_chunk(chunk, seed, town_config);
	if (!chunk->towns.is_empty()) {
		print_line(vformat("[Town] Generated %d towns in Chunk(%d,%d)",
				chunk->towns.size(), x, y));
	}

	chunks[key] = chunk;
	return chunk;
}

void World::print_chunk(int32_t x, int32_t y, int preview_size) {
	Chunk *chunk = get_chunk(x, y);
	print_line(chunk->to_string(preview_size));
}

void World::clear() {
	// 删除所有chunk对象
	for (KeyValue<uint64_t, Chunk *> &kv : chunks) {
		if (kv.value) {
			delete kv.value;
			kv.value = nullptr;
		}
	}
	// 清空HashMap
	chunks.clear();
}

int World::get_town_count(int32_t chunk_x, int32_t chunk_y) {
	Chunk *chunk = get_chunk(chunk_x, chunk_y);
	return chunk ? chunk->towns.size() : 0;
}

Vector<TownInfo> World::get_chunk_towns(int32_t chunk_x, int32_t chunk_y) {
	Chunk *chunk = get_chunk(chunk_x, chunk_y);
	return chunk ? chunk->towns : Vector<TownInfo>();
}
