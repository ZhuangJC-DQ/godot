/**************************************************************************/
/*  road_generator.h                                                      */
/**************************************************************************/

#pragma once

#include "chunk.h"
#include "town_generator.h"
#include "core/math/math_funcs.h"
#include "core/math/vector2i.h"
#include "core/templates/hash_map.h"
#include "core/templates/vector.h"

// 道路生成配置
struct RoadConfig {
	float max_slope = 0.3f; // 最大坡度
	float slope_penalty_factor = 5.0f; // 坡度惩罚系数
	float altitude_penalty_threshold = 0.6f; // 高程惩罚阈值
	float turn_penalty = 0.1f; // 转弯惩罚
	int search_radius_chunks = 2; // MST 搜索半径（Chunk 单位）
	int astar_max_iterations = 5000; // A* 最大迭代次数

	RoadConfig() = default;
};

// RoadSegment 已在 chunk.h 中定义

// 城镇全局唯一标识
struct TownGlobalId {
	ChunkCoord chunk;
	int32_t local_index = 0; // Chunk 内的第几个城镇

	TownGlobalId() = default;
	TownGlobalId(const ChunkCoord &p_chunk, int32_t p_idx) :
			chunk(p_chunk), local_index(p_idx) {}

	uint64_t to_hash() const {
		return ((uint64_t)(uint32_t)chunk.x << 40) |
				((uint64_t)(uint32_t)chunk.y << 16) |
				(uint64_t)(uint16_t)local_index;
	}

	bool operator==(const TownGlobalId &p_other) const {
		return chunk == p_other.chunk && local_index == p_other.local_index;
	}
};

// 虚拟城镇信息（用于道路规划）
struct VirtualTownInfo {
	Vector2i global_tile_pos; // 世界绝对 tile 坐标
	TownGlobalId id;
	float suitability = 0.0f;

	VirtualTownInfo() = default;
	VirtualTownInfo(const Vector2i &p_pos, const TownGlobalId &p_id, float p_suit) :
			global_tile_pos(p_pos), id(p_id), suitability(p_suit) {}
};

// MST 边
struct RoadEdge {
	uint64_t town_a;
	uint64_t town_b;
	float weight;

	RoadEdge() = default;
	RoadEdge(uint64_t p_a, uint64_t p_b, float p_w) :
			town_a(p_a), town_b(p_b), weight(p_w) {}

	bool operator<(const RoadEdge &p_other) const {
		if (Math::abs(weight - p_other.weight) > 0.001f) {
			return weight < p_other.weight;
		}
		// 权重相同时，使用 ID 排序保证确定性
		if (town_a != p_other.town_a) {
			return town_a < p_other.town_a;
		}
		return town_b < p_other.town_b;
	}

	uint64_t get_path_seed() const {
		// 为每条边生成唯一的路径种子
		return town_a ^ (town_b << 1);
	}
};

// 并查集（用于 MST）
class UnionFind {
private:
	HashMap<uint64_t, uint64_t> parent;
	HashMap<uint64_t, int> rank;

public:
	void make_set(uint64_t x);
	uint64_t find(uint64_t x);
	bool union_sets(uint64_t x, uint64_t y);
};

// 道路生成器
class RoadGenerator {
public:
	// 为指定 Chunk 生成道路片段（主入口）
	static Vector<RoadSegment> generate_road_segments_for_chunk(
			int32_t chunk_x, int32_t chunk_y,
			int32_t world_seed,
			const NoiseConfig &noise_config,
			const TownConfig &town_config,
			const RoadConfig &road_config);

	// 虚拟城镇查询（不实际生成 Chunk）
	static Vector<VirtualTownInfo> query_virtual_towns_in_chunk(
			int32_t chunk_x, int32_t chunk_y,
			int32_t world_seed,
			const NoiseConfig &noise_config,
			const TownConfig &town_config);

	// 收集区域内所有虚拟城镇
	static Vector<VirtualTownInfo> collect_virtual_towns_in_region(
			int32_t center_chunk_x, int32_t center_chunk_y,
			int32_t radius_chunks,
			int32_t world_seed,
			const NoiseConfig &noise_config,
			const TownConfig &town_config);

	// 构建最小生成树
	static Vector<RoadEdge> build_mst(
			const Vector<VirtualTownInfo> &towns);

	// A* 寻路（确定性，考虑地形）
	static Vector<Vector2i> find_path_astar(
			const Vector2i &start,
			const Vector2i &end,
			uint64_t path_seed,
			int32_t world_seed,
			const NoiseConfig &noise_config,
			const RoadConfig &road_config);

	// 裁剪路径到 Chunk
	static Vector<Vector2i> clip_path_to_chunk(
			const Vector<Vector2i> &path,
			const ChunkCoord &chunk);

private:
	// 计算地形成本
	static float calculate_terrain_cost(
			float height_from, float height_to,
			const RoadConfig &config);

	// 获取指定位置的高度（虚拟）
	static float get_virtual_height(
			const Vector2i &global_tile_pos,
			int32_t world_seed,
			const NoiseConfig &noise_config);

	// 确定性哈希
	static uint32_t hash_coord(int32_t x, int32_t y, int32_t seed);
	static uint64_t hash_position(const Vector2i &pos, uint64_t seed);
};
