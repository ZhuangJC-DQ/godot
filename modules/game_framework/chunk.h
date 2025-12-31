/**************************************************************************/
/*  chunk.h                                                               */
/**************************************************************************/

#pragma once

#include "world_object.h"
#include "monster.h"
#include "npc.h"

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

// Monster 数据
struct MonsterSpawnData {
	Vector2i position;           // 在 chunk 中的位置
	Ref<Monster> monster;        // 怪物实例
};

// NPC 数据
struct NPCSpawnData {
	Vector2i position;           // 在 chunk 中的位置
	Ref<NPC> npc;                // NPC 实例
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
	// 怪物列表
	Vector<MonsterSpawnData> monsters;
	// NPC 列表
	Vector<NPCSpawnData> npcs;

	Chunk(const ChunkCoord &p_coord);
	void generate();
	void generate_world_objects(RandomPCG &rng);
	void generate_monsters(RandomPCG &rng);
	void generate_npcs(RandomPCG &rng);
	String to_string(int preview_size = 32) const;

	// 获取城市数量
	int get_city_count() const { return cities.size(); }
	// 获取指定城市的 WorldObject
	Ref<WorldObject> get_city_object(int index) const;
	// 打印城市信息
	String city_to_string(int index) const;

	// 获取怪物数量
	int get_monster_count() const { return monsters.size(); }
	// 获取指定怪物
	Ref<Monster> get_monster(int index) const;
	// 打印怪物信息
	String monster_to_string(int index) const;

	// 获取 NPC 数量
	int get_npc_count() const { return npcs.size(); }
	// 获取指定 NPC
	Ref<NPC> get_npc(int index) const;
	// 打印 NPC 信息
	String npc_to_string(int index) const;
};
