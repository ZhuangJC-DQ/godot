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

	world.print_chunk(0, 0, 32);
}

void GameFramework::print_chunk_at(int32_t x, int32_t y) {
	world.print_chunk(x, y, 32);
}
