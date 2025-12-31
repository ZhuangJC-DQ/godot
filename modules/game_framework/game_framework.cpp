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
	ClassDB::bind_method(D_METHOD("print_city", "chunk_x", "chunk_y", "city_index"), &GameFramework::print_city);
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

	// 打印 chunk (0, 0)
	world.print_chunk(0, 0, 32);

	// 获取 chunk 并打印第一个城市的详细信息
	Chunk *chunk = world.get_chunk(0, 0);
	if (chunk && chunk->get_city_count() > 0) {
		print_line("\n========== CITY DETAILS ==========\n");
		print_line(chunk->city_to_string(0));
	}
}

void GameFramework::print_chunk_at(int32_t x, int32_t y) {
	world.print_chunk(x, y, 32);
}

void GameFramework::print_city(int32_t chunk_x, int32_t chunk_y, int32_t city_index) {
	Chunk *chunk = world.get_chunk(chunk_x, chunk_y);
	if (chunk && city_index < chunk->get_city_count()) {
		print_line(chunk->city_to_string(city_index));
	} else {
		print_line(vformat("City index %d not found in chunk (%d, %d)", city_index, chunk_x, chunk_y));
	}
}
