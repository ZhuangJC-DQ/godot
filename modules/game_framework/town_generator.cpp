/**************************************************************************/
/*  town_generator.cpp                                                    */
/**************************************************************************/

#include "town_generator.h"

// 确定性哈希函数 - 用于泊松圆盘采样
uint32_t TownGenerator::hash_coord(int32_t x, int32_t y, int32_t seed) {
	// 使用 FNV-1a 哈希变体
	uint32_t hash = 2166136261u;
	hash ^= (uint32_t)x;
	hash *= 16777619u;
	hash ^= (uint32_t)y;
	hash *= 16777619u;
	hash ^= (uint32_t)seed;
	hash *= 16777619u;
	return hash;
}

// 计算高度统计信息
void TownGenerator::calculate_height_stats(const Chunk *p_chunk, float &r_mean, float &r_variance) {
	float sum = 0.0f;
	float sum_sq = 0.0f;
	const int total = CHUNK_SIZE * CHUNK_SIZE;

	for (int y = 0; y < CHUNK_SIZE; y++) {
		for (int x = 0; x < CHUNK_SIZE; x++) {
			float h = p_chunk->tiles[y][x];
			sum += h;
			sum_sq += h * h;
		}
	}

	r_mean = sum / total;
	r_variance = (sum_sq / total) - (r_mean * r_mean);
}

// 计算 Chunk 适宜度
float TownGenerator::calculate_suitability(const Chunk *p_chunk, const TownConfig &p_config) {
	float mean, variance;
	calculate_height_stats(p_chunk, mean, variance);

	// 高度适宜度：在 [min_height, max_height] 范围内最高
	float height_score = 0.0f;
	if (mean >= p_config.min_height && mean <= p_config.max_height) {
		// 中心点 (min+max)/2 评分最高
		float center = (p_config.min_height + p_config.max_height) * 0.5f;
		float range = (p_config.max_height - p_config.min_height) * 0.5f;
		float dist = Math::abs(mean - center) / range;
		height_score = 1.0f - dist;
	}

	// 平坦度评分：方差越小越好
	float flatness_score = 0.0f;
	if (variance <= p_config.max_variance) {
		flatness_score = 1.0f - (variance / p_config.max_variance);
	}

	// 综合评分 (高度权重 0.6, 平坦度权重 0.4)
	return height_score * 0.6f + flatness_score * 0.4f;
}

// 检查距离约束 - 使用网格化泊松采样
bool TownGenerator::check_distance_constraint(const ChunkCoord &p_coord, const TownConfig &p_config) {
	// 将世界划分为大网格，网格大小 = min_distance_chunks
	// 每个网格最多产生一个城镇候选点
	int grid_size = p_config.min_distance_chunks;

	// 计算当前 Chunk 所在的大网格
	int grid_x = (p_coord.x >= 0) ? (p_coord.x / grid_size) : ((p_coord.x - grid_size + 1) / grid_size);
	int grid_y = (p_coord.y >= 0) ? (p_coord.y / grid_size) : ((p_coord.y - grid_size + 1) / grid_size);

	// 计算在大网格内的相对位置
	int local_x = p_coord.x - grid_x * grid_size;
	int local_y = p_coord.y - grid_y * grid_size;

	// 只有在网格中心位置的 Chunk 才能生成城镇
	// 这保证了城镇间至少有 grid_size/2 的距离
	int center = grid_size / 2;
	return (local_x == center && local_y == center);
}

// 判断是否应该生成城镇
bool TownGenerator::should_spawn_town(const ChunkCoord &p_coord, int32_t p_seed,
		float p_suitability, const TownConfig &p_config) {
	// 1. 检查距离约束（泊松采样的网格化实现）
	if (!check_distance_constraint(p_coord, p_config)) {
		return false;
	}

	// 2. 适宜度阈值检查
	if (p_suitability < 0.3f) {
		return false;
	}

	// 3. 概率性生成（使用确定性哈希）
	uint32_t hash = hash_coord(p_coord.x, p_coord.y, p_seed);
	float random_value = (hash % 10000) / 10000.0f;

	// 适宜度越高，生成概率越大
	float threshold = p_config.spawn_probability * p_suitability;

	return random_value < threshold;
}

// 评估单个位置的适宜度
float TownGenerator::evaluate_location(const Chunk *p_chunk, int32_t tile_x, int32_t tile_y,
		int radius, const TownConfig &p_config) {
	float sum = 0.0f;
	float sum_sq = 0.0f;
	int count = 0;

	// 计算周围区域的高度统计
	for (int dy = -radius; dy <= radius; dy++) {
		for (int dx = -radius; dx <= radius; dx++) {
			int nx = tile_x + dx;
			int ny = tile_y + dy;

			if (nx >= 0 && nx < CHUNK_SIZE && ny >= 0 && ny < CHUNK_SIZE) {
				float h = p_chunk->tiles[ny][nx];
				sum += h;
				sum_sq += h * h;
				count++;
			}
		}
	}

	if (count == 0) {
		return 0.0f;
	}

	float mean = sum / count;
	float variance = (sum_sq / count) - (mean * mean);

	// 与整体适宜度计算类似的评分
	float height_score = 0.0f;
	if (mean >= p_config.min_height && mean <= p_config.max_height) {
		float center = (p_config.min_height + p_config.max_height) * 0.5f;
		float range = (p_config.max_height - p_config.min_height) * 0.5f;
		float dist = Math::abs(mean - center) / range;
		height_score = 1.0f - dist;
	}

	float flatness_score = 0.0f;
	if (variance <= p_config.max_variance) {
		flatness_score = 1.0f - (variance / p_config.max_variance);
	}

	return height_score * 0.5f + flatness_score * 0.5f;
}

// 找到最佳城镇位置
TownInfo TownGenerator::find_best_location(const Chunk *p_chunk, const TownConfig &p_config) {
	TownInfo best;
	float best_score = -1.0f;

	// 采样步长（优化性能，不检查每个 tile）
	const int step = 8;
	const int radius = 16; // 评估半径

	for (int y = radius; y < CHUNK_SIZE - radius; y += step) {
		for (int x = radius; x < CHUNK_SIZE - radius; x += step) {
			float score = evaluate_location(p_chunk, x, y, radius, p_config);

			if (score > best_score) {
				best_score = score;
				best.tile_x = x;
				best.tile_y = y;
			}
		}
	}

	if (best_score > 0.0f) {
		best.chunk_x = p_chunk->coord.x;
		best.chunk_y = p_chunk->coord.y;
		best.suitability = best_score;
		best.valid = true;
	}

	return best;
}
