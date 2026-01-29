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

	// 2. 计算城镇适宜度
	chunk->suitability = TownGenerator::calculate_suitability(chunk, town_config);

	// 3. 判断是否生成城镇
	if (TownGenerator::should_spawn_town(coord, seed, chunk->suitability, town_config)) {
		TownInfo info = TownGenerator::find_best_location(chunk, town_config);
		if (info.valid) {
			chunk->has_town = true;
			chunk->town_tile_x = info.tile_x;
			chunk->town_tile_y = info.tile_y;
			print_line(vformat("[Town] Generated at Chunk(%d,%d) tile(%d,%d) suitability=%.2f",
					x, y, info.tile_x, info.tile_y, info.suitability));
		}
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

bool World::has_town_at(int32_t chunk_x, int32_t chunk_y) {
	Chunk *chunk = get_chunk(chunk_x, chunk_y);
	return chunk && chunk->has_town;
}

TownInfo World::get_town_info(int32_t chunk_x, int32_t chunk_y) {
	Chunk *chunk = get_chunk(chunk_x, chunk_y);
	if (chunk && chunk->has_town) {
		return TownInfo(chunk_x, chunk_y, chunk->town_tile_x, chunk->town_tile_y, chunk->suitability);
	}
	return TownInfo();
}
