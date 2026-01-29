/**************************************************************************/
/*  world.h                                                               */
/**************************************************************************/

#pragma once

#include "chunk.h"
#include "core/templates/hash_map.h"
#include "town_generator.h"

class World {
private:
	HashMap<uint64_t, Chunk *> chunks;
	int32_t seed;
	NoiseConfig noise_config;
	TownConfig town_config;

public:
	World();
	~World();

	void set_seed(int32_t p_seed) { seed = p_seed; }
	int32_t get_seed() const { return seed; }

	void set_noise_config(const NoiseConfig &p_config) { noise_config = p_config; }
	const NoiseConfig &get_noise_config() const { return noise_config; }

	void set_town_config(const TownConfig &p_config) { town_config = p_config; }
	const TownConfig &get_town_config() const { return town_config; }

	Chunk *get_chunk(int32_t x, int32_t y);
	void print_chunk(int32_t x, int32_t y, int preview_size = 32);
	void clear();

	// 城镇查询接口
	bool has_town_at(int32_t chunk_x, int32_t chunk_y);
	TownInfo get_town_info(int32_t chunk_x, int32_t chunk_y);
};
