/**************************************************************************/
/*  world_manager.cpp                                                     */
/**************************************************************************/

#include "world_manager.h"

#include "chunk.h"
#include "core/object/class_db.h"

WorldManager::WorldManager() :
		seed(1337),
		noise_frequency(0.005f),
		noise_octaves(3),
		noise_lacunarity(2.0f),
		noise_gain(0.4f),
		use_terrain_curve(true),
		town_min_height(0.25f),
		town_max_height(0.45f),
		town_min_distance(8) {
	world.set_seed(seed);
	update_noise_config();
	update_town_config();
}

WorldManager::~WorldManager() {
}

void WorldManager::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_seed", "seed"), &WorldManager::set_seed);
	ClassDB::bind_method(D_METHOD("get_seed"), &WorldManager::get_seed);

	ClassDB::bind_method(D_METHOD("set_noise_frequency", "frequency"), &WorldManager::set_noise_frequency);
	ClassDB::bind_method(D_METHOD("get_noise_frequency"), &WorldManager::get_noise_frequency);

	ClassDB::bind_method(D_METHOD("set_noise_octaves", "octaves"), &WorldManager::set_noise_octaves);
	ClassDB::bind_method(D_METHOD("get_noise_octaves"), &WorldManager::get_noise_octaves);

	ClassDB::bind_method(D_METHOD("set_noise_lacunarity", "lacunarity"), &WorldManager::set_noise_lacunarity);
	ClassDB::bind_method(D_METHOD("get_noise_lacunarity"), &WorldManager::get_noise_lacunarity);

	ClassDB::bind_method(D_METHOD("set_noise_gain", "gain"), &WorldManager::set_noise_gain);
	ClassDB::bind_method(D_METHOD("get_noise_gain"), &WorldManager::get_noise_gain);

	ClassDB::bind_method(D_METHOD("set_use_terrain_curve", "use_curve"), &WorldManager::set_use_terrain_curve);
	ClassDB::bind_method(D_METHOD("get_use_terrain_curve"), &WorldManager::get_use_terrain_curve);

	ClassDB::bind_method(D_METHOD("update_all_params", "seed", "frequency", "octaves", "lacunarity", "gain", "use_curve"),
			&WorldManager::update_all_params);

	ClassDB::bind_method(D_METHOD("get_chunk_data", "chunk_x", "chunk_y"), &WorldManager::get_chunk_data);
	ClassDB::bind_method(D_METHOD("get_tile_height", "chunk_x", "chunk_y", "tile_x", "tile_y"), &WorldManager::get_tile_height);

	// 城镇配置方法绑定
	ClassDB::bind_method(D_METHOD("set_town_min_height", "height"), &WorldManager::set_town_min_height);
	ClassDB::bind_method(D_METHOD("get_town_min_height"), &WorldManager::get_town_min_height);
	ClassDB::bind_method(D_METHOD("set_town_max_height", "height"), &WorldManager::set_town_max_height);
	ClassDB::bind_method(D_METHOD("get_town_max_height"), &WorldManager::get_town_max_height);
	ClassDB::bind_method(D_METHOD("set_town_min_distance", "distance"), &WorldManager::set_town_min_distance);
	ClassDB::bind_method(D_METHOD("get_town_min_distance"), &WorldManager::get_town_min_distance);

	// 城镇查询方法绑定 - 新 API
	ClassDB::bind_method(D_METHOD("get_town_count", "chunk_x", "chunk_y"), &WorldManager::get_town_count);
	ClassDB::bind_method(D_METHOD("get_chunk_towns", "chunk_x", "chunk_y"), &WorldManager::get_chunk_towns);
	ClassDB::bind_method(D_METHOD("get_towns_in_range", "center_x", "center_y", "range"), &WorldManager::get_towns_in_range);

	ADD_GROUP("Terrain Generation", "");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "seed"), "set_seed", "get_seed");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "noise_frequency", PROPERTY_HINT_RANGE, "0.001,0.1,0.001"), "set_noise_frequency", "get_noise_frequency");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "noise_octaves", PROPERTY_HINT_RANGE, "1,8,1"), "set_noise_octaves", "get_noise_octaves");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "noise_lacunarity", PROPERTY_HINT_RANGE, "1.0,4.0,0.1"), "set_noise_lacunarity", "get_noise_lacunarity");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "noise_gain", PROPERTY_HINT_RANGE, "0.1,1.0,0.05"), "set_noise_gain", "get_noise_gain");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "use_terrain_curve"), "set_use_terrain_curve", "get_use_terrain_curve");

	ADD_GROUP("Town Generation", "");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "town_min_height", PROPERTY_HINT_RANGE, "0.0,1.0,0.01"), "set_town_min_height", "get_town_min_height");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "town_max_height", PROPERTY_HINT_RANGE, "0.0,1.0,0.01"), "set_town_max_height", "get_town_max_height");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "town_min_distance", PROPERTY_HINT_RANGE, "1,20,1"), "set_town_min_distance", "get_town_min_distance");
}

void WorldManager::set_seed(int32_t p_seed) {
	seed = p_seed;
	world.set_seed(p_seed);
	world.clear();
}

void WorldManager::set_noise_frequency(float p_frequency) {
	noise_frequency = p_frequency;
	update_noise_config();
}

void WorldManager::set_noise_octaves(int32_t p_octaves) {
	noise_octaves = p_octaves;
	update_noise_config();
}

void WorldManager::set_noise_lacunarity(float p_lacunarity) {
	noise_lacunarity = p_lacunarity;
	update_noise_config();
}

void WorldManager::set_noise_gain(float p_gain) {
	noise_gain = p_gain;
	update_noise_config();
}

void WorldManager::set_use_terrain_curve(bool p_use) {
	use_terrain_curve = p_use;
	update_noise_config();
}

void WorldManager::update_all_params(int32_t p_seed, float p_frequency, int32_t p_octaves,
		float p_lacunarity, float p_gain, bool p_use_curve) {
	// 批量更新所有参数，只清除一次chunks
	seed = p_seed;
	noise_frequency = p_frequency;
	noise_octaves = p_octaves;
	noise_lacunarity = p_lacunarity;
	noise_gain = p_gain;
	use_terrain_curve = p_use_curve;

	world.set_seed(seed);

	NoiseConfig config;
	config.frequency = noise_frequency;
	config.octaves = noise_octaves;
	config.lacunarity = noise_lacunarity;
	config.gain = noise_gain;
	config.use_terrain_curve = use_terrain_curve;

	world.set_noise_config(config);
	world.clear(); // 只清除一次
}

void WorldManager::update_noise_config() {
	NoiseConfig config;
	config.frequency = noise_frequency;
	config.octaves = noise_octaves;
	config.lacunarity = noise_lacunarity;
	config.gain = noise_gain;
	config.use_terrain_curve = use_terrain_curve;

	world.set_noise_config(config);
	world.clear(); // 清除旧的chunks，强制重新生成
}

Dictionary WorldManager::get_chunk_data(int32_t chunk_x, int32_t chunk_y) {
	Dictionary data;
	Chunk *chunk = world.get_chunk(chunk_x, chunk_y);
	// world.print_chunk(chunk_x, chunk_y, CHUNK_SIZE);
	if (!chunk) {
		return data;
	}

	data["coord_x"] = chunk_x;
	data["coord_y"] = chunk_y;

	return data;
}

float WorldManager::get_tile_height(int32_t chunk_x, int32_t chunk_y, int32_t tile_x, int32_t tile_y) {
	Chunk *chunk = world.get_chunk(chunk_x, chunk_y);
	if (!chunk || tile_x < 0 || tile_x >= CHUNK_SIZE || tile_y < 0 || tile_y >= CHUNK_SIZE) {
		return -1.0f;
	}

	return chunk->tiles[tile_y][tile_x];
}

// 城镇配置 setter
void WorldManager::set_town_min_height(float p_height) {
	town_min_height = p_height;
	update_town_config();
}

void WorldManager::set_town_max_height(float p_height) {
	town_max_height = p_height;
	update_town_config();
}

void WorldManager::set_town_min_distance(int p_distance) {
	town_min_distance = p_distance;
	update_town_config();
}

void WorldManager::update_town_config() {
	TownConfig config;
	config.min_height = town_min_height;
	config.max_height = town_max_height;
	config.min_distance_tiles = town_min_distance; // 修正：使用 min_distance_tiles
	world.set_town_config(config);
	world.clear();
}

// 城镇查询接口 - 新 API
int WorldManager::get_town_count(int32_t chunk_x, int32_t chunk_y) {
	return world.get_town_count(chunk_x, chunk_y);
}

Array WorldManager::get_chunk_towns(int32_t chunk_x, int32_t chunk_y) {
	Array result;
	Vector<TownInfo> towns = world.get_chunk_towns(chunk_x, chunk_y);

	for (int i = 0; i < towns.size(); i++) {
		Dictionary info;
		info["chunk_x"] = chunk_x;
		info["chunk_y"] = chunk_y;
		info["tile_x"] = towns[i].tile_x;
		info["tile_y"] = towns[i].tile_y;
		info["suitability"] = towns[i].suitability;
		result.push_back(info);
	}

	return result;
}

Array WorldManager::get_towns_in_range(int32_t center_x, int32_t center_y, int range) {
	Array all_towns;
	for (int y = center_y - range; y <= center_y + range; y++) {
		for (int x = center_x - range; x <= center_x + range; x++) {
			Vector<TownInfo> chunk_towns = world.get_chunk_towns(x, y);
			for (int i = 0; i < chunk_towns.size(); i++) {
				Dictionary info;
				info["chunk_x"] = x;
				info["chunk_y"] = y;
				info["tile_x"] = chunk_towns[i].tile_x;
				info["tile_y"] = chunk_towns[i].tile_y;
				info["suitability"] = chunk_towns[i].suitability;
				all_towns.push_back(info);
			}
		}
	}
	return all_towns;
}
