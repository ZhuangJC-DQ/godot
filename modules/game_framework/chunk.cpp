/**************************************************************************/
/*  chunk.cpp                                                             */
/**************************************************************************/

#include "chunk.h"

#include "core/string/print_string.h"
#include "core/variant/variant.h"
#include <cmath>

Chunk::Chunk(const ChunkCoord &p_coord) :
		coord(p_coord), center_x(0), center_y(0) {
	generate();
}

void Chunk::generate() {
	RandomPCG rng(coord.to_seed());

	// 生成随机中心点
	center_x = rng.rand(CHUNK_SIZE);
	center_y = rng.rand(CHUNK_SIZE);

	// 计算最大可能距离（用于归一化）
	float max_dist = std::sqrt((float)(CHUNK_SIZE * CHUNK_SIZE + CHUNK_SIZE * CHUNK_SIZE));

	// 遍历每个格子生成地形
	for (int y = 0; y < CHUNK_SIZE; y++) {
		for (int x = 0; x < CHUNK_SIZE; x++) {
			int dx = x - center_x;
			int dy = y - center_y;
			float dist = std::sqrt((float)(dx * dx + dy * dy));
			float normalized_dist = dist / max_dist;

			// 随机扰动
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
	const char tile_chars[] = {
		'@', // CITY
		'#', // TOWN
		'o', // VILLAGE
		'.', // GRASSLAND
		'T', // FOREST
		'^', // MOUNTAIN
	};

	String result;
	result += vformat("=== Chunk (%d, %d) | Center: (%d, %d) ===\n",
			coord.x, coord.y, center_x, center_y);
	result += "Legend: @ City  # Town  o Village  . Grass  T Forest  ^ Mountain\n";
	result += String("-").repeat(preview_size + 2) + "\n";

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
