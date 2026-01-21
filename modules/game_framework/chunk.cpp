/**************************************************************************/
/*  chunk.cpp                                                             */
/**************************************************************************/

#include "chunk.h"

#include "core/string/print_string.h"
#include "core/variant/variant.h"
#include "thirdparty/misc/FastNoiseLite.h"

Chunk::Chunk(const ChunkCoord &p_coord) :
		coord(p_coord) {
	generate();
}

void Chunk::generate() {
	// 创建 FastNoiseLite 实例
	fastnoiselite::FastNoiseLite noise;

	// 配置噪声参数
	noise.SetSeed(coord.to_seed());
	noise.SetNoiseType(fastnoiselite::FastNoiseLite::NoiseType_Perlin);
	noise.SetFrequency(0.01f); // 控制地形尺度，值越小地形越平缓

	// 配置分形参数（多层噪声叠加）
	noise.SetFractalType(fastnoiselite::FastNoiseLite::FractalType_FBm);
	noise.SetFractalOctaves(4); // 叠加层数
	noise.SetFractalLacunarity(2.0f); // 频率倍数
	noise.SetFractalGain(0.5f); // 振幅衰减

	// 生成地形
	for (int y = 0; y < CHUNK_SIZE; y++) {
		for (int x = 0; x < CHUNK_SIZE; x++) {
			// 计算世界坐标（确保跨区块连续）
			float world_x = coord.x * CHUNK_SIZE + x;
			float world_y = coord.y * CHUNK_SIZE + y;

			// 获取噪声值 [-1.0, 1.0]
			float noise_value = noise.GetNoise(world_x, world_y);

			// 归一化到 [0.0, 1.0]
			float normalized = (noise_value + 1.0f) * 0.5f;

			// 根据噪声值分配地形类型（用于颜色区分）
			TileType type;
			if (normalized < 0.15f) {
				type = TILE_MOUNTAIN; // 深色区域
			} else if (normalized < 0.35f) {
				type = TILE_FOREST; // 中深色区域
			} else if (normalized < 0.60f) {
				type = TILE_GRASSLAND; // 中等区域
			} else if (normalized < 0.75f) {
				type = TILE_VILLAGE; // 中浅色区域
			} else if (normalized < 0.90f) {
				type = TILE_TOWN; // 浅色区域
			} else {
				type = TILE_CITY; // 最浅色区域
			}

			tiles[y][x] = type;
		}
	}
}

String Chunk::to_string(int preview_size) const {
	String result;
	result += vformat("Chunk (%d, %d) - Perlin Noise Terrain\n", coord.x, coord.y);

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
