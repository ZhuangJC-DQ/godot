/**************************************************************************/
/*  town_generator.cpp                                                    */
/**************************************************************************/

#include "town_generator.h"

#include "core/math/random_pcg.h"

// 确定性哈希函数
uint32_t TownGenerator::hash_coord(int32_t x, int32_t y, int32_t seed) {
	uint32_t hash = 2166136261u;
	hash ^= (uint32_t)x;
	hash *= 16777619u;
	hash ^= (uint32_t)y;
	hash *= 16777619u;
	hash ^= (uint32_t)seed;
	hash *= 16777619u;
	return hash;
}

// 在 Chunk 内生成多个城镇（泊松圆盘采样）
Vector<TownInfo> TownGenerator::generate_towns_in_chunk(const Chunk *p_chunk, int32_t p_seed, const TownConfig &p_config) {
	Vector<TownInfo> towns;

	// 使用确定性随机数生成器
	RandomPCG rng;
	uint32_t chunk_seed = hash_coord(p_chunk->coord.x, p_chunk->coord.y, p_seed);
	rng.seed(chunk_seed);

	// Dart-throwing 算法
	for (int attempt = 0; attempt < p_config.max_attempts; attempt++) {
		// 随机选择位置
		int tile_x = rng.rand() % CHUNK_SIZE;
		int tile_y = rng.rand() % CHUNK_SIZE;

		// 检查适宜度
		if (!is_suitable_location(p_chunk, tile_x, tile_y, p_config)) {
			continue;
		}

		// 检查距离约束
		if (!check_distance_to_towns(tile_x, tile_y, towns, p_config.min_distance_tiles)) {
			continue;
		}

		// 计算适宜度评分
		float suitability = calculate_point_suitability(p_chunk, tile_x, tile_y, 16, p_config);

		// 添加城镇
		towns.push_back(TownInfo(tile_x, tile_y, suitability));

		// 达到最大数量
		if (towns.size() >= p_config.max_towns_per_chunk) {
			break;
		}
	}

	return towns;
}

// 检查位置是否适合建城
bool TownGenerator::is_suitable_location(const Chunk *p_chunk, int32_t tile_x, int32_t tile_y, const TownConfig &p_config) {
	// 边界检查（留出评估半径的空间）
	const int margin = 16;
	if (tile_x < margin || tile_x >= CHUNK_SIZE - margin ||
			tile_y < margin || tile_y >= CHUNK_SIZE - margin) {
		return false;
	}

	// 检查高度是否在适宜范围
	float height = p_chunk->tiles[tile_y][tile_x];
	if (height < p_config.min_height || height > p_config.max_height) {
		return false;
	}

	// 检查周围区域的平坦度
	const int radius = 8;
	float sum = 0.0f;
	float sum_sq = 0.0f;
	int count = 0;

	for (int dy = -radius; dy <= radius; dy++) {
		for (int dx = -radius; dx <= radius; dx++) {
			int nx = tile_x + dx;
			int ny = tile_y + dy;
			float h = p_chunk->tiles[ny][nx];
			sum += h;
			sum_sq += h * h;
			count++;
		}
	}

	float mean = sum / count;
	float variance = (sum_sq / count) - (mean * mean);

	return variance <= p_config.max_variance;
}

// 检查与已有城镇的距离约束
bool TownGenerator::check_distance_to_towns(int32_t tile_x, int32_t tile_y,
		const Vector<TownInfo> &existing_towns,
		int min_distance) {
	int min_dist_sq = min_distance * min_distance;

	for (int i = 0; i < existing_towns.size(); i++) {
		const TownInfo &town = existing_towns[i];
		int dx = tile_x - town.tile_x;
		int dy = tile_y - town.tile_y;
		int dist_sq = dx * dx + dy * dy;

		if (dist_sq < min_dist_sq) {
			return false;
		}
	}

	return true;
}

// 计算单点适宜度评分
float TownGenerator::calculate_point_suitability(const Chunk *p_chunk, int32_t tile_x, int32_t tile_y,
		int radius, const TownConfig &p_config) {
	float sum = 0.0f;
	float sum_sq = 0.0f;
	int count = 0;

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

	// 高度适宜度
	float height_score = 0.0f;
	if (mean >= p_config.min_height && mean <= p_config.max_height) {
		float center = (p_config.min_height + p_config.max_height) * 0.5f;
		float range = (p_config.max_height - p_config.min_height) * 0.5f;
		float dist = Math::abs(mean - center) / range;
		height_score = 1.0f - dist;
	}

	// 平坦度评分
	float flatness_score = 0.0f;
	if (variance <= p_config.max_variance) {
		flatness_score = 1.0f - (variance / p_config.max_variance);
	}

	return height_score * 0.5f + flatness_score * 0.5f;
}
