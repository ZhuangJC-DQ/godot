/**************************************************************************/
/*  world_manager.h                                                       */
/**************************************************************************/

#pragma once

#include "scene/main/node.h"
#include "world.h"

// 世界管理器 - 纯数据层，负责区块生成和查询
class WorldManager : public Node {
	GDCLASS(WorldManager, Node);

private:
	World world;
	int32_t seed;

	// 噪声参数
	float noise_frequency;
	int32_t noise_octaves;
	float noise_lacunarity;
	float noise_gain;
	bool use_terrain_curve;

	// 城镇生成参数
	float town_min_height;
	float town_max_height;
	int town_min_distance;

protected:
	static void _bind_methods();

public:
	WorldManager();
	~WorldManager();

	// Seed管理
	void set_seed(int32_t p_seed);
	int32_t get_seed() const { return seed; }

	// 噪声参数管理
	void set_noise_frequency(float p_frequency);
	float get_noise_frequency() const { return noise_frequency; }

	void set_noise_octaves(int32_t p_octaves);
	int32_t get_noise_octaves() const { return noise_octaves; }

	void set_noise_lacunarity(float p_lacunarity);
	float get_noise_lacunarity() const { return noise_lacunarity; }

	void set_noise_gain(float p_gain);
	float get_noise_gain() const { return noise_gain; }

	void set_use_terrain_curve(bool p_use);
	bool get_use_terrain_curve() const { return use_terrain_curve; }

	// 批量更新参数（避免多次清除chunks）
	void update_all_params(int32_t p_seed, float p_frequency, int32_t p_octaves,
			float p_lacunarity, float p_gain, bool p_use_curve);

	void update_noise_config();

	// 城镇配置
	void set_town_min_height(float p_height);
	float get_town_min_height() const { return town_min_height; }

	void set_town_max_height(float p_height);
	float get_town_max_height() const { return town_max_height; }

	void set_town_min_distance(int p_distance);
	int get_town_min_distance() const { return town_min_distance; }

	void update_town_config();

	// 数据查询接口
	Dictionary get_chunk_data(int32_t chunk_x, int32_t chunk_y);
	float get_tile_height(int32_t chunk_x, int32_t chunk_y, int32_t tile_x, int32_t tile_y);

	// 城镇查询接口 - 新 API
	int get_town_count(int32_t chunk_x, int32_t chunk_y);
	Array get_chunk_towns(int32_t chunk_x, int32_t chunk_y);
	Array get_towns_in_range(int32_t center_x, int32_t center_y, int range);

	// C++ 内部访问
	World *get_world() { return &world; }
	Chunk *get_chunk_ptr(int32_t x, int32_t y) { return world.get_chunk(x, y); }
};
