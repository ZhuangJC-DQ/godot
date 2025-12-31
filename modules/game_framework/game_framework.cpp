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
	ClassDB::bind_method(D_METHOD("print_monster", "chunk_x", "chunk_y", "monster_index"), &GameFramework::print_monster);
	ClassDB::bind_method(D_METHOD("print_npc", "chunk_x", "chunk_y", "npc_index"), &GameFramework::print_npc);
	ClassDB::bind_method(D_METHOD("print_all_entities", "chunk_x", "chunk_y"), &GameFramework::print_all_entities);
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

	// 获取 chunk 并打印详细信息
	Chunk *chunk = world.get_chunk(0, 0);
	if (chunk) {
		// 打印第一个城市
		if (chunk->get_city_count() > 0) {
			print_line("\n========== CITY DETAILS ==========\n");
			print_line(chunk->city_to_string(0));
		}

		// 打印第一个怪物
		if (chunk->get_monster_count() > 0) {
			print_line("\n========== MONSTER DETAILS ==========\n");
			print_line(chunk->monster_to_string(0));
		}

		// 打印第一个 NPC
		if (chunk->get_npc_count() > 0) {
			print_line("\n========== NPC DETAILS ==========\n");
			print_line(chunk->npc_to_string(0));
		}
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

void GameFramework::print_monster(int32_t chunk_x, int32_t chunk_y, int32_t monster_index) {
	Chunk *chunk = world.get_chunk(chunk_x, chunk_y);
	if (chunk && monster_index < chunk->get_monster_count()) {
		print_line(chunk->monster_to_string(monster_index));
	} else {
		print_line(vformat("Monster index %d not found in chunk (%d, %d)", monster_index, chunk_x, chunk_y));
	}
}

void GameFramework::print_npc(int32_t chunk_x, int32_t chunk_y, int32_t npc_index) {
	Chunk *chunk = world.get_chunk(chunk_x, chunk_y);
	if (chunk && npc_index < chunk->get_npc_count()) {
		print_line(chunk->npc_to_string(npc_index));
	} else {
		print_line(vformat("NPC index %d not found in chunk (%d, %d)", npc_index, chunk_x, chunk_y));
	}
}

void GameFramework::print_all_entities(int32_t chunk_x, int32_t chunk_y) {
	Chunk *chunk = world.get_chunk(chunk_x, chunk_y);
	if (!chunk) {
		print_line(vformat("Chunk (%d, %d) not found", chunk_x, chunk_y));
		return;
	}

	print_line(vformat("\n========== ALL ENTITIES IN CHUNK (%d, %d) ==========\n", chunk_x, chunk_y));

	// 打印所有城市
	print_line(vformat("--- CITIES (%d) ---\n", chunk->get_city_count()));
	for (int i = 0; i < chunk->get_city_count(); i++) {
		print_line(chunk->city_to_string(i));
	}

	// 打印所有怪物
	print_line(vformat("\n--- MONSTERS (%d) ---\n", chunk->get_monster_count()));
	for (int i = 0; i < chunk->get_monster_count(); i++) {
		print_line(chunk->monster_to_string(i));
	}

	// 打印所有 NPC
	print_line(vformat("\n--- NPCs (%d) ---\n", chunk->get_npc_count()));
	for (int i = 0; i < chunk->get_npc_count(); i++) {
		print_line(chunk->npc_to_string(i));
	}
}
