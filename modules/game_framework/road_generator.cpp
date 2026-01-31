/**************************************************************************/
/*  road_generator.cpp                                                    */
/**************************************************************************/

#include "road_generator.h"

#include "core/math/math_funcs.h"
#include "core/math/random_pcg.h"
#include "core/os/os.h"
#include "core/string/print_string.h"
#include "thirdparty/misc/FastNoiseLite.h"
#include "town_generator.h"

// ========== 并查集实现 ==========
void UnionFind::make_set(uint64_t x) {
	if (!parent.has(x)) {
		parent[x] = x;
		rank[x] = 0;
	}
}

uint64_t UnionFind::find(uint64_t x) {
	if (!parent.has(x)) {
		make_set(x);
	}
	if (parent[x] != x) {
		parent[x] = find(parent[x]); // 路径压缩
	}
	return parent[x];
}

bool UnionFind::union_sets(uint64_t x, uint64_t y) {
	uint64_t px = find(x);
	uint64_t py = find(y);

	if (px == py) {
		return false; // 已在同一集合
	}

	// 按秩合并
	if (rank[px] < rank[py]) {
		parent[px] = py;
	} else if (rank[px] > rank[py]) {
		parent[py] = px;
	} else {
		parent[py] = px;
		rank[px]++;
	}
	return true;
}

// ========== RoadGenerator 实现 ==========

// 确定性哈希函数
uint32_t RoadGenerator::hash_coord(int32_t x, int32_t y, int32_t seed) {
	uint32_t hash = 2166136261u;
	hash ^= (uint32_t)x;
	hash *= 16777619u;
	hash ^= (uint32_t)y;
	hash *= 16777619u;
	hash ^= (uint32_t)seed;
	hash *= 16777619u;
	return hash;
}

uint64_t RoadGenerator::hash_position(const Vector2i &pos, uint64_t seed) {
	uint64_t hash = 14695981039346656037ULL;
	hash ^= (uint64_t)pos.x;
	hash *= 1099511628211ULL;
	hash ^= (uint64_t)pos.y;
	hash *= 1099511628211ULL;
	hash ^= seed;
	hash *= 1099511628211ULL;
	return hash;
}

// 虚拟查询单个 Chunk 的城镇（重现生成算法）
Vector<VirtualTownInfo> RoadGenerator::query_virtual_towns_in_chunk(
		int32_t chunk_x, int32_t chunk_y,
		int32_t world_seed,
		const NoiseConfig &noise_config,
		const TownConfig &town_config) {
	Vector<VirtualTownInfo> virtual_towns;

	// 创建虚拟高度图
	fastnoiselite::FastNoiseLite noise;
	noise.SetSeed(world_seed);
	noise.SetNoiseType(fastnoiselite::FastNoiseLite::NoiseType_Perlin);
	noise.SetFrequency(noise_config.frequency);
	noise.SetFractalType(fastnoiselite::FastNoiseLite::FractalType_FBm);
	noise.SetFractalOctaves(noise_config.octaves);
	noise.SetFractalLacunarity(noise_config.lacunarity);
	noise.SetFractalGain(noise_config.gain);

	// 使用确定性随机数生成器
	RandomPCG rng;
	uint32_t chunk_seed = hash_coord(chunk_x, chunk_y, world_seed);
	rng.seed(chunk_seed);

	Vector<TownInfo> local_towns;

	// Dart-throwing 算法（与 TownGenerator 完全一致）
	for (int attempt = 0; attempt < town_config.max_attempts; attempt++) {
		int tile_x = rng.rand() % CHUNK_SIZE;
		int tile_y = rng.rand() % CHUNK_SIZE;

		// 边界检查
		const int margin = 16;
		if (tile_x < margin || tile_x >= CHUNK_SIZE - margin ||
				tile_y < margin || tile_y >= CHUNK_SIZE - margin) {
			continue;
		}

		// 计算全局坐标
		int global_x = chunk_x * CHUNK_SIZE + tile_x;
		int global_y = chunk_y * CHUNK_SIZE + tile_y;

		// 获取高度
		float raw_noise = noise.GetNoise((float)global_x, (float)global_y);
		float height = (raw_noise + 1.0f) * 0.5f; // [-1, 1] -> [0, 1]

		if (noise_config.use_terrain_curve) {
			height = Math::pow(height, 2.5f);
		}

		// 高度检查
		if (height < town_config.min_height || height > town_config.max_height) {
			continue;
		}

		// 平坦度检查（简化版，仅检查周围）
		const int radius = 8;
		float sum = 0.0f;
		float sum_sq = 0.0f;
		int count = 0;

		for (int dy = -radius; dy <= radius; dy++) {
			for (int dx = -radius; dx <= radius; dx++) {
				int gx = global_x + dx;
				int gy = global_y + dy;
				float rn = noise.GetNoise((float)gx, (float)gy);
				float h = (rn + 1.0f) * 0.5f;
				if (noise_config.use_terrain_curve) {
					h = Math::pow(h, 2.5f);
				}
				sum += h;
				sum_sq += h * h;
				count++;
			}
		}

		float mean = sum / count;
		float variance = (sum_sq / count) - (mean * mean);

		if (variance > town_config.max_variance) {
			continue;
		}

		// 距离检查
		bool too_close = false;
		int min_dist_sq = town_config.min_distance_tiles * town_config.min_distance_tiles;
		for (int i = 0; i < local_towns.size(); i++) {
			int dx = tile_x - local_towns[i].tile_x;
			int dy = tile_y - local_towns[i].tile_y;
			if (dx * dx + dy * dy < min_dist_sq) {
				too_close = true;
				break;
			}
		}

		if (too_close) {
			continue;
		}

		// 计算适宜度
		float height_score = 0.0f;
		if (mean >= town_config.min_height && mean <= town_config.max_height) {
			float center = (town_config.min_height + town_config.max_height) * 0.5f;
			float range = (town_config.max_height - town_config.min_height) * 0.5f;
			float dist = Math::abs(mean - center) / range;
			height_score = 1.0f - dist;
		}

		float flatness_score = 0.0f;
		if (variance <= town_config.max_variance) {
			flatness_score = 1.0f - (variance / town_config.max_variance);
		}

		float suitability = height_score * 0.5f + flatness_score * 0.5f;

		local_towns.push_back(TownInfo(tile_x, tile_y, suitability));

		if (local_towns.size() >= town_config.max_towns_per_chunk) {
			break;
		}
	}

	// 转换为虚拟城镇信息
	for (int i = 0; i < local_towns.size(); i++) {
		Vector2i global_pos(
				chunk_x * CHUNK_SIZE + local_towns[i].tile_x,
				chunk_y * CHUNK_SIZE + local_towns[i].tile_y);

		TownGlobalId id(ChunkCoord(chunk_x, chunk_y), i);
		virtual_towns.push_back(VirtualTownInfo(global_pos, id, local_towns[i].suitability));
	}

	return virtual_towns;
}

// 收集区域内所有虚拟城镇
Vector<VirtualTownInfo> RoadGenerator::collect_virtual_towns_in_region(
		int32_t center_chunk_x, int32_t center_chunk_y,
		int32_t radius_chunks,
		int32_t world_seed,
		const NoiseConfig &noise_config,
		const TownConfig &town_config) {
	Vector<VirtualTownInfo> all_towns;

	for (int32_t dy = -radius_chunks; dy <= radius_chunks; dy++) {
		for (int32_t dx = -radius_chunks; dx <= radius_chunks; dx++) {
			Vector<VirtualTownInfo> chunk_towns = query_virtual_towns_in_chunk(
					center_chunk_x + dx,
					center_chunk_y + dy,
					world_seed,
					noise_config,
					town_config);
			all_towns.append_array(chunk_towns);
		}
	}

	return all_towns;
}

// 构建最小生成树（Kruskal 算法）
Vector<RoadEdge> RoadGenerator::build_mst(const Vector<VirtualTownInfo> &towns) {
	if (towns.size() < 2) {
		return Vector<RoadEdge>();
	}

	// 生成所有边
	Vector<RoadEdge> edges;
	for (int i = 0; i < towns.size(); i++) {
		for (int j = i + 1; j < towns.size(); j++) {
			float dist = towns[i].global_tile_pos.distance_to(towns[j].global_tile_pos);
			edges.push_back(RoadEdge(towns[i].id.to_hash(), towns[j].id.to_hash(), dist));
		}
	}

	// 排序（确定性）
	edges.sort();

	// Kruskal 算法
	UnionFind uf;
	for (int i = 0; i < towns.size(); i++) {
		uf.make_set(towns[i].id.to_hash());
	}

	Vector<RoadEdge> mst;
	for (int i = 0; i < edges.size(); i++) {
		if (uf.union_sets(edges[i].town_a, edges[i].town_b)) {
			mst.push_back(edges[i]);
			if (mst.size() >= towns.size() - 1) {
				break; // MST 完成
			}
		}
	}

	return mst;
}

// 获取虚拟高度
float RoadGenerator::get_virtual_height(
		const Vector2i &global_tile_pos,
		int32_t world_seed,
		const NoiseConfig &noise_config) {
	fastnoiselite::FastNoiseLite noise;
	noise.SetSeed(world_seed);
	noise.SetNoiseType(fastnoiselite::FastNoiseLite::NoiseType_Perlin);
	noise.SetFrequency(noise_config.frequency);
	noise.SetFractalType(fastnoiselite::FastNoiseLite::FractalType_FBm);
	noise.SetFractalOctaves(noise_config.octaves);
	noise.SetFractalLacunarity(noise_config.lacunarity);
	noise.SetFractalGain(noise_config.gain);

	float raw = noise.GetNoise((float)global_tile_pos.x, (float)global_tile_pos.y);
	float height = (raw + 1.0f) * 0.5f;

	if (noise_config.use_terrain_curve) {
		height = Math::pow(height, 2.5f);
	}

	return height;
}

// 计算地形成本
float RoadGenerator::calculate_terrain_cost(
		float height_from, float height_to,
		const RoadConfig &config) {
	float base_cost = 1.0f;
	float height_diff = Math::abs(height_to - height_from);

	// 坡度惩罚
	float slope_penalty = Math::pow(height_diff / config.max_slope, 2.0f) * config.slope_penalty_factor;

	// 高程惩罚
	float altitude_penalty = 0.0f;
	float avg_height = (height_from + height_to) * 0.5f;
	if (avg_height > config.altitude_penalty_threshold) {
		altitude_penalty = (avg_height - config.altitude_penalty_threshold) * 5.0f;
	}

	return base_cost + slope_penalty + altitude_penalty;
}

// A* 节点
struct AStarNode {
	Vector2i pos;
	float g_score = 1e38f; // 使用大数代替无穷大
	float f_score = 1e38f;
	Vector2i parent = Vector2i(-1, -1);

	AStarNode() = default;
	AStarNode(const Vector2i &p_pos) :
			pos(p_pos) {}
};

// A* 确定性比较器
struct AStarComparator {
	uint64_t seed;

	AStarComparator(uint64_t p_seed) :
			seed(p_seed) {}

	bool operator()(const AStarNode *a, const AStarNode *b) const {
		if (Math::abs(a->f_score - b->f_score) < 0.001f) {
			// F 值相同，使用位置哈希排序
			uint64_t hash_a = ((uint64_t)(uint32_t)a->pos.x << 32) | (uint64_t)(uint32_t)a->pos.y;
			uint64_t hash_b = ((uint64_t)(uint32_t)b->pos.x << 32) | (uint64_t)(uint32_t)b->pos.y;
			hash_a ^= seed;
			hash_b ^= seed;
			return hash_a > hash_b; // 大顶堆需要反向
		}
		return a->f_score > b->f_score;
	}
};

// A* 寻路（确定性）
Vector<Vector2i> RoadGenerator::find_path_astar(
		const Vector2i &start,
		const Vector2i &end,
		uint64_t path_seed,
		int32_t world_seed,
		const NoiseConfig &noise_config,
		const RoadConfig &road_config) {
	HashMap<Vector2i, AStarNode> nodes;
	Vector<AStarNode *> open_set;
	HashMap<Vector2i, bool> closed_set;

	// 初始化起点
	AStarNode start_node(start);
	start_node.g_score = 0.0f;
	start_node.f_score = start.distance_to(end);
	nodes[start] = start_node;
	open_set.push_back(&nodes[start]);

	AStarComparator comparator(path_seed);

	int iterations = 0;
	const int max_iter = road_config.astar_max_iterations;

	while (!open_set.is_empty() && iterations < max_iter) {
		iterations++;

		// 获取 F 值最小的节点（使用确定性比较）
		int min_idx = 0;
		for (int i = 1; i < open_set.size(); i++) {
			if (comparator(open_set[min_idx], open_set[i])) {
				min_idx = i;
			}
		}

		AStarNode *current = open_set[min_idx];
		open_set.remove_at(min_idx);

		// 到达终点
		if (current->pos == end) {
			Vector<Vector2i> path;
			Vector2i p = end;
			while (p != Vector2i(-1, -1)) {
				path.push_back(p);
				if (nodes.has(p)) {
					p = nodes[p].parent;
				} else {
					break;
				}
			}
			path.reverse();
			return path;
		}

		closed_set[current->pos] = true;

		// 扩展邻居（8方向）
		const Vector2i dirs[8] = {
			Vector2i(1, 0), Vector2i(-1, 0), Vector2i(0, 1), Vector2i(0, -1),
			Vector2i(1, 1), Vector2i(1, -1), Vector2i(-1, 1), Vector2i(-1, -1)
		};

		for (int i = 0; i < 8; i++) {
			Vector2i neighbor_pos = current->pos + dirs[i];

			if (closed_set.has(neighbor_pos)) {
				continue;
			}

			// 获取高度
			float height_current = get_virtual_height(current->pos, world_seed, noise_config);
			float height_neighbor = get_virtual_height(neighbor_pos, world_seed, noise_config);

			// 计算成本
			float move_cost = (i < 4) ? 1.0f : 1.414f; // 直线或对角线
			float terrain_cost = calculate_terrain_cost(height_current, height_neighbor, road_config);
			float tentative_g = current->g_score + move_cost * terrain_cost;

			// 更新或创建节点
			if (!nodes.has(neighbor_pos)) {
				nodes[neighbor_pos] = AStarNode(neighbor_pos);
			}

			AStarNode &neighbor = nodes[neighbor_pos];

			if (tentative_g < neighbor.g_score) {
				neighbor.parent = current->pos;
				neighbor.g_score = tentative_g;
				neighbor.f_score = tentative_g + neighbor_pos.distance_to(end);

				// 加入 open_set
				bool in_open = false;
				for (int j = 0; j < open_set.size(); j++) {
					if (open_set[j]->pos == neighbor_pos) {
						in_open = true;
						break;
					}
				}

				if (!in_open) {
					open_set.push_back(&neighbor);
				}
			}
		}
	}

	// 未找到路径，返回直线
	Vector<Vector2i> fallback;
	fallback.push_back(start);
	fallback.push_back(end);
	return fallback;
}

// 裁剪路径到 Chunk
Vector<Vector2i> RoadGenerator::clip_path_to_chunk(
		const Vector<Vector2i> &path,
		const ChunkCoord &chunk) {
	Vector<Vector2i> local_segment;

	int chunk_min_x = chunk.x * CHUNK_SIZE;
	int chunk_max_x = (chunk.x + 1) * CHUNK_SIZE - 1;
	int chunk_min_y = chunk.y * CHUNK_SIZE;
	int chunk_max_y = (chunk.y + 1) * CHUNK_SIZE - 1;

	for (int i = 0; i < path.size(); i++) {
		if (path[i].x >= chunk_min_x && path[i].x <= chunk_max_x &&
				path[i].y >= chunk_min_y && path[i].y <= chunk_max_y) {
			// 转换为 Chunk 内坐标
			Vector2i local_pos(
					path[i].x - chunk_min_x,
					path[i].y - chunk_min_y);
			local_segment.push_back(local_pos);
		}
	}

	return local_segment;
}

// 为指定 Chunk 生成道路片段（主入口）
Vector<RoadSegment> RoadGenerator::generate_road_segments_for_chunk(
		int32_t chunk_x, int32_t chunk_y,
		int32_t world_seed,
		const NoiseConfig &noise_config,
		const TownConfig &town_config,
		const RoadConfig &road_config) {
	Vector<RoadSegment> segments;

	uint64_t t_start = OS::get_singleton()->get_ticks_usec();

	// 1. 收集周围区域的虚拟城镇
	uint64_t t1 = OS::get_singleton()->get_ticks_usec();
	Vector<VirtualTownInfo> region_towns = collect_virtual_towns_in_region(
			chunk_x, chunk_y,
			road_config.search_radius_chunks,
			world_seed,
			noise_config,
			town_config);
	uint64_t t2 = OS::get_singleton()->get_ticks_usec();
	print_line(vformat("  [Road] Collect towns: %.2f ms (%d towns)", (t2 - t1) / 1000.0, region_towns.size()));

	if (region_towns.size() < 2) {
		print_line("  [Road] Insufficient towns, skip road generation");
		return segments;
	}

	// 2. 构建 MST
	uint64_t t3 = OS::get_singleton()->get_ticks_usec();
	Vector<RoadEdge> mst = build_mst(region_towns);
	uint64_t t4 = OS::get_singleton()->get_ticks_usec();
	print_line(vformat("  [Road] Build MST: %.2f ms (%d edges)", (t4 - t3) / 1000.0, mst.size()));

	// 3. 对每条 MST 边生成路径并裁剪
	uint64_t t5 = OS::get_singleton()->get_ticks_usec();
	int astar_count = 0;
	int skipped_count = 0;

	int chunk_min_x = chunk_x * CHUNK_SIZE;
	int chunk_max_x = (chunk_x + 1) * CHUNK_SIZE - 1;
	int chunk_min_y = chunk_y * CHUNK_SIZE;
	int chunk_max_y = (chunk_y + 1) * CHUNK_SIZE - 1;

	for (int i = 0; i < mst.size(); i++) {
		const RoadEdge &edge = mst[i];

		// 查找城镇位置
		Vector2i pos_a, pos_b;
		bool found_a = false, found_b = false;

		for (int j = 0; j < region_towns.size(); j++) {
			if (region_towns[j].id.to_hash() == edge.town_a) {
				pos_a = region_towns[j].global_tile_pos;
				found_a = true;
			}
			if (region_towns[j].id.to_hash() == edge.town_b) {
				pos_b = region_towns[j].global_tile_pos;
				found_b = true;
			}
			if (found_a && found_b) {
				break;
			}
		}

		if (!found_a || !found_b) {
			continue;
		}

		// 粗略判断：路径是否可能穿过当前Chunk（包围盒检测）
		int min_x = MIN(pos_a.x, pos_b.x);
		int max_x = MAX(pos_a.x, pos_b.x);
		int min_y = MIN(pos_a.y, pos_b.y);
		int max_y = MAX(pos_a.y, pos_b.y);

		if (max_x < chunk_min_x || min_x > chunk_max_x ||
				max_y < chunk_min_y || min_y > chunk_max_y) {
			skipped_count++;
			continue;
		}

		// A* 寻路
		astar_count++;
		Vector<Vector2i> path = find_path_astar(
				pos_a, pos_b,
				edge.get_path_seed(),
				world_seed,
				noise_config,
				road_config);

		// 裁剪到当前 Chunk
		Vector<Vector2i> local_segment = clip_path_to_chunk(path, ChunkCoord(chunk_x, chunk_y));

		if (!local_segment.is_empty()) {
			segments.push_back(RoadSegment(local_segment, edge.town_a, edge.town_b, edge.weight));
		}
	}

	uint64_t t6 = OS::get_singleton()->get_ticks_usec();
	uint64_t t_total = t6 - t_start;
	print_line(vformat("  [Road] A* pathfinding: %.2f ms (ran %d, skipped %d)", (t6 - t5) / 1000.0, astar_count, skipped_count));
	print_line(vformat("  [Road] TOTAL generation: %.2f ms -> %d segments", t_total / 1000.0, segments.size()));

	return segments;
}
