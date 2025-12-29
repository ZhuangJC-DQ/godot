/**************************************************************************/
/*  game_framework.cpp                                                    */
/**************************************************************************/

#include "game_framework.h"

#include "core/string/print_string.h"

GameFramework::GameFramework() {
}

GameFramework::~GameFramework() {
}

void GameFramework::_bind_methods() {
	ClassDB::bind_method(D_METHOD("test_map_generation"), &GameFramework::test_map_generation);
	ClassDB::bind_method(D_METHOD("print_chunk_at", "x", "y"), &GameFramework::print_chunk_at);
}

void GameFramework::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
			_ready();
		} break;
	}
}

void GameFramework::_ready() {
	print_line("GameFramework: Ready!");
	test_map_generation();
}

void GameFramework::test_map_generation() {
	print_line("\n========== MAP GENERATION TEST ==========\n");

	// 测试几个不同坐标的区块
	world.print_chunk(0, 0, 32);
	world.print_chunk(1, 0, 32);
	world.print_chunk(0, 1, 32);

	// 验证确定性：相同坐标应该生成相同地图
	print_line("=== Determinism Test: Chunk (0,0) again ===");
	Chunk *c1 = world.get_chunk(0, 0);
	print_line(vformat("Center should be same: (%d, %d)", c1->center_x, c1->center_y));

	print_line("\n========== TEST COMPLETE ==========\n");
}

void GameFramework::print_chunk_at(int32_t x, int32_t y) {
	world.print_chunk(x, y, 32);
}
