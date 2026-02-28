/**************************************************************************/
/*  test_character.h                                                      */
/**************************************************************************/

#pragma once

#include "../character/game_character.h"
#include "../character/character_manager.h"

#include "tests/test_macros.h"

namespace TestCharacter {

TEST_CASE("[GameCharacter] Default construction") {
	GameCharacter c;
	CHECK(c.get_id() == 0);
	CHECK(c.get_type() == CHARACTER_TYPE_PLAYER);
	CHECK(c.get_level() == 1);
	CHECK(c.get_hp() == 100);
	CHECK(c.get_max_hp() == 100);
	CHECK(c.get_mp() == 50);
	CHECK(c.get_max_mp() == 50);
	CHECK(c.get_attack() == 10);
	CHECK(c.get_defense() == 5);
	CHECK(c.is_alive() == true);
}

TEST_CASE("[GameCharacter] Parameterized construction") {
	GameCharacter c(42, CHARACTER_TYPE_ENEMY);
	CHECK(c.get_id() == 42);
	CHECK(c.get_type() == CHARACTER_TYPE_ENEMY);
}

TEST_CASE("[GameCharacter] HP clamping") {
	GameCharacter c(1, CHARACTER_TYPE_PLAYER);

	c.set_hp(200);
	CHECK(c.get_hp() == 100); // Clamped to max_hp

	c.set_hp(-10);
	CHECK(c.get_hp() == 0);
	CHECK(c.is_alive() == false); // Died at 0 HP
}

TEST_CASE("[GameCharacter] Max HP adjustment") {
	GameCharacter c(1, CHARACTER_TYPE_PLAYER);
	c.set_hp(80);

	c.set_max_hp(50);
	CHECK(c.get_max_hp() == 50);
	CHECK(c.get_hp() == 50); // HP clamped to new max

	c.set_max_hp(0);
	CHECK(c.get_max_hp() == 1); // Min max_hp is 1
}

TEST_CASE("[GameCharacter] MP clamping") {
	GameCharacter c(1, CHARACTER_TYPE_PLAYER);

	c.set_mp(200);
	CHECK(c.get_mp() == 50); // Clamped to max_mp

	c.set_mp(-5);
	CHECK(c.get_mp() == 0);
}

TEST_CASE("[GameCharacter] Take damage with defense") {
	GameCharacter c(1, CHARACTER_TYPE_PLAYER);
	c.set_defense(3);
	c.set_hp(100);
	c.set_max_hp(100);

	int actual = c.take_damage(10);
	CHECK(actual == 7); // 10 - 3 defense
	CHECK(c.get_hp() == 93);

	// Damage less than defense
	actual = c.take_damage(2);
	CHECK(actual == 0); // Minimum 0 damage
	CHECK(c.get_hp() == 93);
}

TEST_CASE("[GameCharacter] Lethal damage") {
	GameCharacter c(1, CHARACTER_TYPE_PLAYER);
	c.set_defense(0);
	c.set_hp(10);

	c.take_damage(100);
	CHECK(c.get_hp() == 0);
	CHECK(c.is_alive() == false);
}

TEST_CASE("[GameCharacter] Heal") {
	GameCharacter c(1, CHARACTER_TYPE_PLAYER);
	c.set_hp(50);

	c.heal(20);
	CHECK(c.get_hp() == 70);

	c.heal(100);
	CHECK(c.get_hp() == 100); // Clamped to max_hp
}

TEST_CASE("[GameCharacter] Heal when dead") {
	GameCharacter c(1, CHARACTER_TYPE_PLAYER);
	c.set_defense(0);
	c.set_hp(1);
	c.take_damage(10);
	CHECK(c.is_alive() == false);

	c.heal(50);
	CHECK(c.get_hp() == 0); // Cannot heal when dead
}

TEST_CASE("[GameCharacter] Custom properties") {
	GameCharacter c(1, CHARACTER_TYPE_NPC);

	CHECK(c.has_custom_property("quest_giver") == false);

	c.set_custom_property("quest_giver", true);
	CHECK(c.has_custom_property("quest_giver") == true);
	CHECK(c.get_custom_property("quest_giver") == Variant(true));

	c.remove_custom_property("quest_giver");
	CHECK(c.has_custom_property("quest_giver") == false);
}

TEST_CASE("[CharacterManager] Create and destroy") {
	CharacterManager manager;

	uint64_t id = manager.create_character(CHARACTER_TYPE_PLAYER);
	CHECK(id > 0);
	CHECK(manager.is_valid_character(id));
	CHECK(manager.get_character_count() == 1);
	CHECK(manager.get_character_type(id) == CHARACTER_TYPE_PLAYER);

	manager.destroy_character(id);
	CHECK(!manager.is_valid_character(id));
	CHECK(manager.get_character_count() == 0);
}

TEST_CASE("[CharacterManager] Create with ID") {
	CharacterManager manager;

	uint64_t id = manager.create_character_with_id(100, CHARACTER_TYPE_NPC);
	CHECK(id == 100);
	CHECK(manager.is_valid_character(100));

	// Duplicate ID should fail
	ERR_PRINT_OFF;
	uint64_t dup = manager.create_character_with_id(100, CHARACTER_TYPE_ENEMY);
	ERR_PRINT_ON;
	CHECK(dup == 0);
}

TEST_CASE("[CharacterManager] Invalid type") {
	CharacterManager manager;

	ERR_PRINT_OFF;
	uint64_t id = manager.create_character(-1);
	CHECK(id == 0);
	id = manager.create_character(99);
	CHECK(id == 0);
	ERR_PRINT_ON;
}

TEST_CASE("[CharacterManager] Property getters and setters") {
	CharacterManager manager;
	uint64_t id = manager.create_character(CHARACTER_TYPE_PLAYER);

	manager.set_character_name(id, "Hero");
	CHECK(manager.get_character_name(id) == "Hero");

	manager.set_level(id, 5);
	CHECK(manager.get_level(id) == 5);

	manager.set_hp(id, 80);
	CHECK(manager.get_hp(id) == 80);

	manager.set_max_hp(id, 200);
	CHECK(manager.get_max_hp(id) == 200);

	manager.set_attack(id, 25);
	CHECK(manager.get_attack(id) == 25);

	manager.set_defense(id, 15);
	CHECK(manager.get_defense(id) == 15);

	manager.set_speed(id, 8.5f);
	CHECK(manager.get_speed(id) == doctest::Approx(8.5f));
}

TEST_CASE("[CharacterManager] Inventory and equipment IDs") {
	CharacterManager manager;
	uint64_t id = manager.create_character(CHARACTER_TYPE_PLAYER);

	CHECK(manager.get_inventory_id(id) == 0);
	CHECK(manager.get_equipment_id(id) == 0);

	manager.set_inventory_id(id, 42);
	manager.set_equipment_id(id, 43);

	CHECK(manager.get_inventory_id(id) == 42);
	CHECK(manager.get_equipment_id(id) == 43);
}

TEST_CASE("[CharacterManager] Combat operations") {
	CharacterManager manager;
	uint64_t id = manager.create_character(CHARACTER_TYPE_PLAYER);
	manager.set_defense(id, 3);

	int actual = manager.take_damage(id, 10);
	CHECK(actual == 7);
	CHECK(manager.get_hp(id) == 93);

	manager.heal(id, 5);
	CHECK(manager.get_hp(id) == 98);
}

TEST_CASE("[CharacterManager] Get characters by type") {
	CharacterManager manager;

	uint64_t p1 = manager.create_character(CHARACTER_TYPE_PLAYER);
	uint64_t n1 = manager.create_character(CHARACTER_TYPE_NPC);
	uint64_t n2 = manager.create_character(CHARACTER_TYPE_NPC);
	uint64_t e1 = manager.create_character(CHARACTER_TYPE_ENEMY);

	TypedArray<int> players = manager.get_characters_by_type(CHARACTER_TYPE_PLAYER);
	CHECK(players.size() == 1);

	TypedArray<int> npcs = manager.get_characters_by_type(CHARACTER_TYPE_NPC);
	CHECK(npcs.size() == 2);

	TypedArray<int> enemies = manager.get_characters_by_type(CHARACTER_TYPE_ENEMY);
	CHECK(enemies.size() == 1);

	// Suppress unused variable warnings
	(void)p1;
	(void)n1;
	(void)n2;
	(void)e1;
}

TEST_CASE("[CharacterManager] Batch data operations") {
	CharacterManager manager;
	uint64_t id = manager.create_character(CHARACTER_TYPE_PLAYER);
	manager.set_character_name(id, "TestHero");
	manager.set_level(id, 10);
	manager.set_attack(id, 30);

	Dictionary data = manager.get_character_data(id);
	CHECK(data["name"] == Variant("TestHero"));
	CHECK(data["level"] == Variant(10));
	CHECK(data["attack"] == Variant(30));
	CHECK(data["type"] == Variant(CHARACTER_TYPE_PLAYER));
}

TEST_CASE("[CharacterManager] Set character data") {
	CharacterManager manager;
	uint64_t id = manager.create_character(CHARACTER_TYPE_PLAYER);

	Dictionary data;
	data["name"] = "DataHero";
	data["level"] = 20;
	data["max_hp"] = 500;
	data["hp"] = 400;
	data["attack"] = 50;

	manager.set_character_data(id, data);
	CHECK(manager.get_character_name(id) == "DataHero");
	CHECK(manager.get_level(id) == 20);
	CHECK(manager.get_max_hp(id) == 500);
	CHECK(manager.get_hp(id) == 400);
	CHECK(manager.get_attack(id) == 50);
}

TEST_CASE("[CharacterManager] Save and load") {
	CharacterManager manager;

	uint64_t p1 = manager.create_character(CHARACTER_TYPE_PLAYER);
	manager.set_character_name(p1, "SaveHero");
	manager.set_level(p1, 15);
	manager.set_attack(p1, 40);

	uint64_t n1 = manager.create_character(CHARACTER_TYPE_NPC);
	manager.set_character_name(n1, "ShopKeeper");

	Dictionary save_data = manager.save_to_dict();

	// Clear and reload into the same manager
	manager.load_from_dict(save_data);

	CHECK(manager.get_character_count() == 2);
	CHECK(manager.is_valid_character(p1));
	CHECK(manager.get_character_name(p1) == "SaveHero");
	CHECK(manager.get_level(p1) == 15);
	CHECK(manager.get_attack(p1) == 40);
	CHECK(manager.get_character_type(p1) == CHARACTER_TYPE_PLAYER);

	CHECK(manager.is_valid_character(n1));
	CHECK(manager.get_character_name(n1) == "ShopKeeper");
	CHECK(manager.get_character_type(n1) == CHARACTER_TYPE_NPC);
}

TEST_CASE("[CharacterManager] Custom properties via manager") {
	CharacterManager manager;
	uint64_t id = manager.create_character(CHARACTER_TYPE_NPC);

	manager.set_custom_property(id, "dialogue_id", 42);
	CHECK(manager.has_custom_property(id, "dialogue_id") == true);
	CHECK(manager.get_custom_property(id, "dialogue_id") == Variant(42));

	manager.remove_custom_property(id, "dialogue_id");
	CHECK(manager.has_custom_property(id, "dialogue_id") == false);
}

TEST_CASE("[CharacterManager] Clear all characters") {
	CharacterManager manager;

	manager.create_character(CHARACTER_TYPE_PLAYER);
	manager.create_character(CHARACTER_TYPE_NPC);
	manager.create_character(CHARACTER_TYPE_ENEMY);
	CHECK(manager.get_character_count() == 3);

	manager.clear_all_characters();
	CHECK(manager.get_character_count() == 0);
}

TEST_CASE("[CharacterManager] Invalid character operations") {
	CharacterManager manager;

	ERR_PRINT_OFF;
	CHECK(manager.get_character_name(999) == "");
	CHECK(manager.get_hp(999) == 0);
	CHECK(manager.is_alive(999) == false);
	CHECK(manager.get_character_type(999) == -1);
	ERR_PRINT_ON;
}

} // namespace TestCharacter
