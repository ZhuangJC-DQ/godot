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
	String result;
	result += vformat("Chunk (%d, %d) - Center: (%d, %d)\n", coord.x, coord.y, center_x, center_y);

	// 限制预览大小
	int size = MIN(preview_size, CHUNK_SIZE);
	int start_x = (CHUNK_SIZE - size) / 2;
	int start_y = (CHUNK_SIZE - size) / 2;

	// 打印地形预览
	for (int y = start_y; y < start_y + size; y++) {
		for (int x = start_x; x < start_x + size; x++) {
			char tile_char;
			switch (tiles[y][x]) {
				case TILE_CITY:
					tile_char = 'C';
					break;
				case TILE_TOWN:
					tile_char = 'T';
					break;
				case TILE_VILLAGE:
					tile_char = 'V';
					break;
				case TILE_GRASSLAND:
					tile_char = '.';
					break;
				case TILE_FOREST:
					tile_char = '#';
					break;
				case TILE_MOUNTAIN:
					tile_char = '^';
					break;
				default:
					tile_char = '?';
					break;
			}
			result += tile_char;
		}
		result += "\n";
	}

	return result;
}
