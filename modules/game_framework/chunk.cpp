/**************************************************************************/
/*  chunk.cpp                                                             */
/**************************************************************************/

#include "chunk.h"

#include "core/string/print_string.h"
#include "core/variant/variant.h"
#include "thirdparty/misc/FastNoiseLite.h"

Chunk::Chunk(const ChunkCoord &p_coord, int32_t p_seed, const NoiseConfig &p_config) :
		coord(p_coord) {
	generate(p_seed, p_config);
}

void Chunk::generate(int32_t p_seed, const NoiseConfig &p_config) {
	// 创建 FastNoiseLite 实例
	fastnoiselite::FastNoiseLite noise;

	// 配置噪声参数 - 使用传入的参数
	noise.SetSeed(p_seed);
	noise.SetNoiseType(fastnoiselite::FastNoiseLite::NoiseType_Perlin);
	noise.SetFrequency(p_config.frequency);

	// 配置分形参数（多层噪声叠加）
	noise.SetFractalType(fastnoiselite::FastNoiseLite::FractalType_FBm);
	noise.SetFractalOctaves(p_config.octaves);
	noise.SetFractalLacunarity(p_config.lacunarity);
	noise.SetFractalGain(p_config.gain);

	// 生成高度图 - 优化：减少函数调用，提前计算基础坐标
	int base_x = coord.x * CHUNK_SIZE;
	int base_y = coord.y * CHUNK_SIZE;
	
	for (int y = 0; y < CHUNK_SIZE; y++) {
		float world_y = base_y + y;
		for (int x = 0; x < CHUNK_SIZE; x++) {
			float world_x = base_x + x;

			// 获取噪声值 [-1.0, 1.0] 并归一化到 [0.0, 1.0]
			float height = (noise.GetNoise(world_x, world_y) + 1.0f) * 0.5f;

			// 可选：使用幂函数创建更多平原区域
			if (p_config.use_terrain_curve) {
				if (height < 0.5f) {
					// 低地变更平
					height = height * height * 2.0f; // 平方后缩放回[0, 0.5]
				} else {
					// 高地保持起伏
					float t = (height - 0.5f) * 2.0f; // [0.5, 1.0] -> [0, 1.0]
					height = 0.5f + t * t * 0.5f; // 平方后映射到[0.5, 1.0]
				}
			}

			tiles[y][x] = height;
		}
	}
}

String Chunk::to_string(int preview_size) const {
	String result;
	result += vformat("Chunk (%d, %d) - Perlin Noise Height Map\n", coord.x, coord.y);

	// 限制预览大小
	int size = MIN(preview_size, CHUNK_SIZE);
	int start_x = (CHUNK_SIZE - size) / 2;
	int start_y = (CHUNK_SIZE - size) / 2;

	// 打印高度图预览
	for (int y = start_y; y < start_y + size; y++) {
		for (int x = start_x; x < start_x + size; x++) {
			// 将高度值 [0.0, 1.0] 映射到 ASCII 灰度字符
			float height = tiles[y][x];
			const char* gradient = " .:-=+*#%@";
			int index = (int)(height * 9.0f);
			if (index > 9) index = 9;
			result += gradient[index];
		}
		result += "\n";
	}

	return result;
}
