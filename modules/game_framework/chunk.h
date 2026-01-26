/**************************************************************************/
/*  chunk.h                                                               */
/**************************************************************************/

#pragma once

#include "core/math/random_pcg.h"
#include "core/string/ustring.h"

// 区块常量
constexpr int CHUNK_SIZE = 256;

// 噪声配置
struct NoiseConfig {
	float frequency = 0.005f;
	int32_t octaves = 3;
	float lacunarity = 2.0f;
	float gain = 0.4f;
	bool use_terrain_curve = true; // 是否使用高度曲线（创建平原）

	NoiseConfig() = default;
};

// 区块坐标
struct ChunkCoord {
	int32_t x;
	int32_t y;

	ChunkCoord(int32_t p_x = 0, int32_t p_y = 0) :
			x(p_x), y(p_y) {}

	uint64_t to_seed() const {
		return ((uint64_t)(uint32_t)x << 32) | (uint64_t)(uint32_t)y;
	}

	bool operator==(const ChunkCoord &p_other) const {
		return x == p_other.x && y == p_other.y;
	}
};

// 区块数据 - 使用柏林噪声生成高度图
class Chunk {
public:
	ChunkCoord coord;
	float tiles[CHUNK_SIZE][CHUNK_SIZE]; // 存储归一化高度值 [0.0, 1.0]

	Chunk(const ChunkCoord &p_coord, int32_t p_seed = 1337, const NoiseConfig &p_config = NoiseConfig());
	void generate(int32_t p_seed, const NoiseConfig &p_config);
	String to_string(int preview_size = 32) const;
};
