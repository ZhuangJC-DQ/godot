/**************************************************************************/
/*  game_framework.h                                                      */
/**************************************************************************/

#pragma once

#include "scene/main/node.h"
#include "world.h"


class GameFramework : public Node {
	GDCLASS(GameFramework, Node);

private:
	World world;

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
};
