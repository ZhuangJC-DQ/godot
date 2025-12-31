/**************************************************************************/
/*  chunk.h                                                               */
/**************************************************************************/

#pragma once

#include "world_object.h"

#include "core/math/random_pcg.h"
#include "core/string/ustring.h"
#include "core/templates/vector.h"

// 地块类型
enum TileType {
	TILE_CITY,      // 城市中心
	TILE_TOWN,      // 城镇
	TILE_VILLAGE,   // 村庄
	TILE_GRASSLAND, // 草原
	TILE_FOREST,    // 树林
	TILE_MOUNTAIN,  // 山地
};

// 区块常量
constexpr int CHUNK_SIZE = 256;

// 区块坐标
struct ChunkCoord {
	int32_t x;
	int32_t y;

	ChunkCoord(int32_t p_x = 0, int32_t p_y = 0) :
			x(p_x), y(p_y) {}

	uint64_t to_seed() const {
		return ((uint64_t)(uint32_t)x << 32) | (uint64_t)(uint32_t)y;
	}

	bool operator==(const ChunkCoord &p_other) const {
		return x == p_other.x && y == p_other.y;
	}
};

// 城市数据
struct CityData {
	Vector2i position;           // 在 chunk 中的位置
	Ref<WorldObject> world_object;  // 关联的 WorldObject
};

// 区块数据
class Chunk {
public:
	ChunkCoord coord;
	TileType tiles[CHUNK_SIZE][CHUNK_SIZE];
	int center_x;
	int center_y;

	// 城市列表
	Vector<CityData> cities;

	Chunk(const ChunkCoord &p_coord);
	void generate();
	void generate_world_objects(RandomPCG &rng);
	String to_string(int preview_size = 32) const;

	// 获取城市数量
	int get_city_count() const { return cities.size(); }
	// 获取指定城市的 WorldObject
	Ref<WorldObject> get_city_object(int index) const;
	// 打印城市信息
	String city_to_string(int index) const;
};
