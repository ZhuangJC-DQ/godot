/**************************************************************************/
/*  world.cpp                                                             */
/**************************************************************************/

#include "world.h"

#include "core/string/print_string.h"
#include "core/variant/variant.h"
#include <cmath>

// ==================== Chunk 实现 ====================

Chunk::Chunk(const ChunkCoord &p_coord) :
		coord(p_coord), center_x(0), center_y(0) {
	generate();
}

void Chunk::generate() {
	// 使用 Godot 内置的 PCG 随机数生成器
	RandomPCG rng(coord.to_seed());

	// 生成随机中心点
	center_x = rng.rand(CHUNK_SIZE);
	center_y = rng.rand(CHUNK_SIZE);

	// 计算最大可能距离（用于归一化）
	float max_dist = std::sqrt((float)(CHUNK_SIZE * CHUNK_SIZE + CHUNK_SIZE * CHUNK_SIZE));

	// 遍历每个格子生成地形
	for (int y = 0; y < CHUNK_SIZE; y++) {
		for (int x = 0; x < CHUNK_SIZE; x++) {
			// 计算到中心点的距离
			int dx = x - center_x;
			int dy = y - center_y;
			float dist = std::sqrt((float)(dx * dx + dy * dy));
			float normalized_dist = dist / max_dist; // 0.0 ~ 1.0

			// 加入一些随机扰动
			float noise = rng.randf() * 0.15f;
			normalized_dist += noise;

			// 根据距离决定地形类型
			TileType type;
			if (normalized_dist < 0.08f) {
				type = TILE_CITY;
			} else if (normalized_dist < 0.15f) {
				type = TILE_TOWN;
			} else if (normalized_dist < 0.25f) {
				type = TILE_VILLAGE;
			} else if (normalized_dist < 0.45f) {
				type = TILE_GRASSLAND;
			} else if (normalized_dist < 0.70f) {
				type = TILE_FOREST;
			} else {
				type = TILE_MOUNTAIN;
			}

			tiles[y][x] = type;
		}
	}
}

String Chunk::to_string(int preview_size) const {
	// 字符映射
	const char tile_chars[] = {
		'@', // CITY - 城市
		'#', // TOWN - 城镇
		'o', // VILLAGE - 村庄
		'.', // GRASSLAND - 草原
		'T', // FOREST - 树林
		'^', // MOUNTAIN - 山地
	};

	String result;
	result += vformat("=== Chunk (%d, %d) | Center: (%d, %d) ===\n",
			coord.x, coord.y, center_x, center_y);
	result += "Legend: @ City  # Town  o Village  . Grass  T Forest  ^ Mountain\n";
	result += String("-").repeat(preview_size + 2) + "\n";

	// 采样显示（如果 preview_size < CHUNK_SIZE 则采样）
	int step = CHUNK_SIZE / preview_size;
	for (int y = 0; y < preview_size; y++) {
		result += "|";
		for (int x = 0; x < preview_size; x++) {
			int sample_x = x * step;
			int sample_y = y * step;
			TileType type = tiles[sample_y][sample_x];
			result += tile_chars[type];
		}
		result += "|\n";
	}

	result += String("-").repeat(preview_size + 2) + "\n";
	return result;
}

// ==================== World 实现 ====================

World::World() {
}

World::~World() {
	clear();
}

Chunk *World::get_chunk(int32_t x, int32_t y) {
	ChunkCoord coord(x, y);
	uint64_t key = coord.to_seed();

	// 查找已存在的区块
	if (chunks.has(key)) {
		return chunks[key];
	}

	// 生成新区块
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
