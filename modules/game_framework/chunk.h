/**************************************************************************/
/*  chunk.h                                                               */
/**************************************************************************/

#pragma once

#include "core/math/random_pcg.h"
#include "core/math/vector2i.h"
#include "core/string/ustring.h"
#include "core/templates/vector.h"

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

// 城镇信息（仅存储 Chunk 内位置）
struct TownInfo {
	int32_t tile_x = 0; // Chunk 内 tile 坐标
	int32_t tile_y = 0;
	float suitability = 0.0f; // 适宜度评分 [0, 1]

	TownInfo() = default;
	TownInfo(int32_t tx, int32_t ty, float suit) :
			tile_x(tx), tile_y(ty), suitability(suit) {}
};

// 道路片段 - 存储在 Chunk 中
struct RoadSegment {
	Vector<Vector2i> tiles; // Chunk 内的 tile 坐标序列
	uint64_t town_a_id = 0; // 起点城镇 ID
	uint64_t town_b_id = 0; // 终点城镇 ID
	float total_cost = 0.0f; // 总成本

	RoadSegment() = default;
	RoadSegment(const Vector<Vector2i> &p_tiles, uint64_t p_a, uint64_t p_b, float p_cost = 0.0f) :
			tiles(p_tiles), town_a_id(p_a), town_b_id(p_b), total_cost(p_cost) {}
};

// 前向声明（移除）
// struct RoadSegment;

// 区块数据 - 使用柏林噪声生成高度图
class Chunk {
public:
	ChunkCoord coord;
	float tiles[CHUNK_SIZE][CHUNK_SIZE]; // 存储归一化高度值 [0.0, 1.0]

	// 城镇数据 - 支持多个城镇
	Vector<TownInfo> towns;

	// 道路数据 - 穿过此 Chunk 的道路片段
	Vector<RoadSegment> road_segments;
	bool roads_generated = false; // 标记是否已生成道路

	Chunk(const ChunkCoord &p_coord, int32_t p_seed = 1337, const NoiseConfig &p_config = NoiseConfig());
	void generate(int32_t p_seed, const NoiseConfig &p_config);
	String to_string(int preview_size = 32) const;
};
