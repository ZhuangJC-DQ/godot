/**************************************************************************/
/*  game_framework.h                                                      */
/**************************************************************************/

#pragma once

#include "scene/3d/node_3d.h"
#include "world.h"

class GameFramework : public Node3D {
	GDCLASS(GameFramework, Node3D);

private:
	World world;

	// 可视化相关
	float tile_size = 1.0f; // 每个格子的大小（米）
	Node3D *chunk_visual = nullptr;

	void create_chunk_visual(int32_t chunk_x, int32_t chunk_y);
	void create_terrain_mesh(Chunk *chunk);
	void create_entity_visuals(Chunk *chunk);
	Color get_tile_color(TileType type) const;

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	GameFramework();
	~GameFramework();

	void _ready();

	void test_map_generation();
	void print_chunk_at(int32_t x, int32_t y);
	void print_city(int32_t chunk_x, int32_t chunk_y, int32_t city_index);
	void print_monster(int32_t chunk_x, int32_t chunk_y, int32_t monster_index);
	void print_npc(int32_t chunk_x, int32_t chunk_y, int32_t npc_index);
	void print_all_entities(int32_t chunk_x, int32_t chunk_y);

	// 可视化方法
	void visualize_chunk(int32_t chunk_x, int32_t chunk_y);
	void clear_visualization();
	void set_tile_size(float p_size) { tile_size = p_size; }
	float get_tile_size() const { return tile_size; }

	// 编辑器支持
	void generate_in_editor();
};
