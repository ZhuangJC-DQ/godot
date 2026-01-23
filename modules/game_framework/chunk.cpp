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

	// 生成高度图
	for (int y = 0; y < CHUNK_SIZE; y++) {
		for (int x = 0; x < CHUNK_SIZE; x++) {
			// 计算世界坐标（确保跨区块连续）
			float world_x = coord.x * CHUNK_SIZE + x;
			float world_y = coord.y * CHUNK_SIZE + y;

			// 获取噪声值 [-1.0, 1.0]
			float noise_value = noise.GetNoise(world_x, world_y);

			// 归一化到 [0.0, 1.0] 作为高度值
			tiles[y][x] = (noise_value + 1.0f) * 0.5f;
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
