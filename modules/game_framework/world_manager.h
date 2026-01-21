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

protected:
	static void _bind_methods();

public:
	WorldManager();
	~WorldManager();

	// 数据查询接口
	Dictionary get_chunk_data(int32_t chunk_x, int32_t chunk_y);
	int get_tile_type(int32_t chunk_x, int32_t chunk_y, int32_t tile_x, int32_t tile_y);

	// C++ 内部访问
	World *get_world() { return &world; }
	Chunk *get_chunk_ptr(int32_t x, int32_t y) { return world.get_chunk(x, y); }
};
