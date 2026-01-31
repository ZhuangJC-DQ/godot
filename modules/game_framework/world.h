/**************************************************************************/
/*  world.h                                                               */
/**************************************************************************/

#pragma once

#include "chunk.h"
#include "core/templates/hash_map.h"
#include "road_generator.h"
#include "town_generator.h"

class World {
private:
	HashMap<uint64_t, Chunk *> chunks;
	int32_t seed;
	NoiseConfig noise_config;
	TownConfig town_config;
	RoadConfig road_config;

public:
	World();
	~World();

	void set_seed(int32_t p_seed) { seed = p_seed; }
	int32_t get_seed() const { return seed; }

	void set_noise_config(const NoiseConfig &p_config) { noise_config = p_config; }
	const NoiseConfig &get_noise_config() const { return noise_config; }

	void set_town_config(const TownConfig &p_config) { town_config = p_config; }
	const TownConfig &get_town_config() const { return town_config; }

	void set_road_config(const RoadConfig &p_config) { road_config = p_config; }
	const RoadConfig &get_road_config() const { return road_config; }

	Chunk *get_chunk(int32_t x, int32_t y);
	void print_chunk(int32_t x, int32_t y, int preview_size = 32);
	void clear();

	// 城镇查询接口
	int get_town_count(int32_t chunk_x, int32_t chunk_y);
	Vector<TownInfo> get_chunk_towns(int32_t chunk_x, int32_t chunk_y);

	// 道路查询接口
	int get_road_count(int32_t chunk_x, int32_t chunk_y);
	Vector<RoadSegment> get_chunk_roads(int32_t chunk_x, int32_t chunk_y);
};
