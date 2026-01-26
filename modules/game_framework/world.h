/**************************************************************************/
/*  world.h                                                               */
/**************************************************************************/

#pragma once

#include "chunk.h"
#include "core/templates/hash_map.h"

class World {
private:
	HashMap<uint64_t, Chunk *> chunks;
	int32_t seed;
	NoiseConfig noise_config;

public:
	World();
	~World();

	void set_seed(int32_t p_seed) { seed = p_seed; }
	int32_t get_seed() const { return seed; }
	
	void set_noise_config(const NoiseConfig &p_config) { noise_config = p_config; }
	const NoiseConfig &get_noise_config() const { return noise_config; }

	Chunk *get_chunk(int32_t x, int32_t y);
	void print_chunk(int32_t x, int32_t y, int preview_size = 32);
	void clear();
};
