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
	int min_distance_tiles = 32; // 城镇间最小距离（tile单位）
	int max_towns_per_chunk = 10; // 每个Chunk最大城镇数
	int max_attempts = 500; // 泊松采样最大尝试次数

	TownConfig() = default;
};

// 城镇生成器 - 基于高度图 + 泊松圆盘采样
class TownGenerator {
public:
	// 在 Chunk 内生成多个城镇（主入口）
	static Vector<TownInfo> generate_towns_in_chunk(const Chunk *p_chunk, int32_t p_seed, const TownConfig &p_config);

	// 检查位置是否适合建城
	static bool is_suitable_location(const Chunk *p_chunk, int32_t tile_x, int32_t tile_y, const TownConfig &p_config);

	// 检查与已有城镇的距离约束
	static bool check_distance_to_towns(int32_t tile_x, int32_t tile_y,
			const Vector<TownInfo> &existing_towns,
			int min_distance);

	// 计算单点适宜度
	static float calculate_point_suitability(const Chunk *p_chunk, int32_t tile_x, int32_t tile_y,
			int radius, const TownConfig &p_config);

private:
	// 确定性哈希函数
	static uint32_t hash_coord(int32_t x, int32_t y, int32_t seed);
};
