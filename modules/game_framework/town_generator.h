/**************************************************************************/
/*  town_generator.h                                                      */
/**************************************************************************/

#pragma once

#include "chunk.h"

// 城镇生成配置
struct TownConfig {
	float min_height = 0.25f; // 最低适宜高度
	float max_height = 0.45f; // 最高适宜高度
	float max_variance = 0.05f; // 最大高度方差（平坦度要求）
	int min_distance_chunks = 8; // 城镇间最小距离（以Chunk为单位）
	float spawn_probability = 0.3f; // 满足条件时的生成概率

	TownConfig() = default;
};

// 城镇信息
struct TownInfo {
	int32_t chunk_x = 0;
	int32_t chunk_y = 0;
	int32_t tile_x = 0; // Chunk 内 tile 坐标
	int32_t tile_y = 0;
	float suitability = 0.0f; // 适宜度评分 [0, 1]
	bool valid = false;

	TownInfo() = default;
	TownInfo(int32_t cx, int32_t cy, int32_t tx, int32_t ty, float suit) :
			chunk_x(cx), chunk_y(cy), tile_x(tx), tile_y(ty), suitability(suit), valid(true) {}
};

// 城镇生成器 - 基于高度图 + 泊松圆盘采样
class TownGenerator {
public:
	// 计算 Chunk 的城镇适宜度 (基于平均高度和方差)
	// 返回 [0, 1] 范围的适宜度评分
	static float calculate_suitability(const Chunk *p_chunk, const TownConfig &p_config);

	// 计算 Chunk 的高度统计信息
	static void calculate_height_stats(const Chunk *p_chunk, float &r_mean, float &r_variance);

	// 确定性判断此 Chunk 是否应该生成城镇
	// 使用坐标哈希 + 泊松圆盘采样思想保证跨 Chunk 一致性
	static bool should_spawn_town(const ChunkCoord &p_coord, int32_t p_seed,
			float p_suitability, const TownConfig &p_config);

	// 在 Chunk 内找到最佳城镇位置 (最平坦的区域中心)
	static TownInfo find_best_location(const Chunk *p_chunk, const TownConfig &p_config);

	// 评估单个城镇候选点的分数
	// 使用周围区域的平均高度和方差
	static float evaluate_location(const Chunk *p_chunk, int32_t tile_x, int32_t tile_y,
			int radius, const TownConfig &p_config);

private:
	// 确定性哈希函数，用于泊松圆盘采样
	static uint32_t hash_coord(int32_t x, int32_t y, int32_t seed);

	// 检查与其他潜在城镇的最小距离约束
	// 使用网格化泊松采样：将世界划分为大网格，每个网格最多一个城镇
	static bool check_distance_constraint(const ChunkCoord &p_coord, const TownConfig &p_config);
};
