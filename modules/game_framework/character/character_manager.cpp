/**************************************************************************/
/*  character_manager.cpp                                                 */
/**************************************************************************/

#include "character_manager.h"

#include "core/object/class_db.h"
#include "core/string/print_string.h"
#include "core/variant/variant.h"

CharacterManager *CharacterManager::singleton = nullptr;

CharacterManager *CharacterManager::get_singleton() {
	return singleton;
}

CharacterManager::CharacterManager() {
	ERR_FAIL_COND_MSG(singleton != nullptr, "CharacterManager singleton already exists!");
	singleton = this;
}

CharacterManager::~CharacterManager() {
	clear_all_characters();
	singleton = nullptr;
}

void CharacterManager::_bind_methods() {
	// === 角色生命周期 ===
	ClassDB::bind_method(D_METHOD("create_character", "type"), &CharacterManager::create_character);
	ClassDB::bind_method(D_METHOD("create_character_with_id", "id", "type"), &CharacterManager::create_character_with_id);
	ClassDB::bind_method(D_METHOD("destroy_character", "character_id"), &CharacterManager::destroy_character);
	ClassDB::bind_method(D_METHOD("is_valid_character", "character_id"), &CharacterManager::is_valid_character);
	ClassDB::bind_method(D_METHOD("get_character_count"), &CharacterManager::get_character_count);
	ClassDB::bind_method(D_METHOD("get_all_character_ids"), &CharacterManager::get_all_character_ids);
	ClassDB::bind_method(D_METHOD("get_characters_by_type", "type"), &CharacterManager::get_characters_by_type);

	// === 基础属性 ===
	ClassDB::bind_method(D_METHOD("get_character_name", "character_id"), &CharacterManager::get_character_name);
	ClassDB::bind_method(D_METHOD("set_character_name", "character_id", "name"), &CharacterManager::set_character_name);
	ClassDB::bind_method(D_METHOD("get_character_type", "character_id"), &CharacterManager::get_character_type);
	ClassDB::bind_method(D_METHOD("get_level", "character_id"), &CharacterManager::get_level);
	ClassDB::bind_method(D_METHOD("set_level", "character_id", "level"), &CharacterManager::set_level);
	ClassDB::bind_method(D_METHOD("get_hp", "character_id"), &CharacterManager::get_hp);
	ClassDB::bind_method(D_METHOD("set_hp", "character_id", "hp"), &CharacterManager::set_hp);
	ClassDB::bind_method(D_METHOD("get_max_hp", "character_id"), &CharacterManager::get_max_hp);
	ClassDB::bind_method(D_METHOD("set_max_hp", "character_id", "max_hp"), &CharacterManager::set_max_hp);
	ClassDB::bind_method(D_METHOD("get_mp", "character_id"), &CharacterManager::get_mp);
	ClassDB::bind_method(D_METHOD("set_mp", "character_id", "mp"), &CharacterManager::set_mp);
	ClassDB::bind_method(D_METHOD("get_max_mp", "character_id"), &CharacterManager::get_max_mp);
	ClassDB::bind_method(D_METHOD("set_max_mp", "character_id", "max_mp"), &CharacterManager::set_max_mp);

	// === 战斗属性 ===
	ClassDB::bind_method(D_METHOD("get_attack", "character_id"), &CharacterManager::get_attack);
	ClassDB::bind_method(D_METHOD("set_attack", "character_id", "attack"), &CharacterManager::set_attack);
	ClassDB::bind_method(D_METHOD("get_defense", "character_id"), &CharacterManager::get_defense);
	ClassDB::bind_method(D_METHOD("set_defense", "character_id", "defense"), &CharacterManager::set_defense);
	ClassDB::bind_method(D_METHOD("get_speed", "character_id"), &CharacterManager::get_speed);
	ClassDB::bind_method(D_METHOD("set_speed", "character_id", "speed"), &CharacterManager::set_speed);

	// === 装备/背包 ===
	ClassDB::bind_method(D_METHOD("get_inventory_id", "character_id"), &CharacterManager::get_inventory_id);
	ClassDB::bind_method(D_METHOD("set_inventory_id", "character_id", "inventory_id"), &CharacterManager::set_inventory_id);
	ClassDB::bind_method(D_METHOD("get_equipment_id", "character_id"), &CharacterManager::get_equipment_id);
	ClassDB::bind_method(D_METHOD("set_equipment_id", "character_id", "equipment_id"), &CharacterManager::set_equipment_id);

	// === 状态与战斗 ===
	ClassDB::bind_method(D_METHOD("is_alive", "character_id"), &CharacterManager::is_alive);
	ClassDB::bind_method(D_METHOD("set_alive", "character_id", "alive"), &CharacterManager::set_alive);
	ClassDB::bind_method(D_METHOD("take_damage", "character_id", "damage"), &CharacterManager::take_damage);
	ClassDB::bind_method(D_METHOD("heal", "character_id", "amount"), &CharacterManager::heal);

	// === 自定义属性 ===
	ClassDB::bind_method(D_METHOD("set_custom_property", "character_id", "key", "value"), &CharacterManager::set_custom_property);
	ClassDB::bind_method(D_METHOD("get_custom_property", "character_id", "key"), &CharacterManager::get_custom_property);
	ClassDB::bind_method(D_METHOD("has_custom_property", "character_id", "key"), &CharacterManager::has_custom_property);
	ClassDB::bind_method(D_METHOD("remove_custom_property", "character_id", "key"), &CharacterManager::remove_custom_property);

	// === 批量操作 ===
	ClassDB::bind_method(D_METHOD("get_character_data", "character_id"), &CharacterManager::get_character_data);
	ClassDB::bind_method(D_METHOD("set_character_data", "character_id", "data"), &CharacterManager::set_character_data);

	// === 序列化 ===
	ClassDB::bind_method(D_METHOD("save_to_dict"), &CharacterManager::save_to_dict);
	ClassDB::bind_method(D_METHOD("load_from_dict", "data"), &CharacterManager::load_from_dict);

	// === 调试 ===
	ClassDB::bind_method(D_METHOD("print_character", "character_id"), &CharacterManager::print_character);
	ClassDB::bind_method(D_METHOD("print_all_characters"), &CharacterManager::print_all_characters);
	ClassDB::bind_method(D_METHOD("clear_all_characters"), &CharacterManager::clear_all_characters);

	// === 常量 ===
	BIND_CONSTANT(CHARACTER_TYPE_PLAYER);
	BIND_CONSTANT(CHARACTER_TYPE_NPC);
	BIND_CONSTANT(CHARACTER_TYPE_ENEMY);
}

// ============================================
// === 角色生命周期 ===
// ============================================

uint64_t CharacterManager::create_character(int p_type) {
	ERR_FAIL_COND_V(p_type < 0 || p_type >= CHARACTER_TYPE_MAX, 0);

	uint64_t id = next_id++;
	GameCharacter *character = new GameCharacter(id, static_cast<CharacterType>(p_type));
	characters[id] = character;

	print_line(vformat("[CharacterManager] Created character: id=%d, type=%d", id, p_type));
	return id;
}

uint64_t CharacterManager::create_character_with_id(uint64_t p_id, int p_type) {
	ERR_FAIL_COND_V_MSG(characters.has(p_id), 0, vformat("Character with ID %d already exists!", p_id));
	ERR_FAIL_COND_V(p_type < 0 || p_type >= CHARACTER_TYPE_MAX, 0);

	GameCharacter *character = new GameCharacter(p_id, static_cast<CharacterType>(p_type));
	characters[p_id] = character;

	if (p_id >= next_id) {
		next_id = p_id + 1;
	}

	return p_id;
}

void CharacterManager::destroy_character(uint64_t p_character_id) {
	GameCharacter *character = get_character(p_character_id);
	ERR_FAIL_COND(!character);

	characters.erase(p_character_id);
	delete character;
}

bool CharacterManager::is_valid_character(uint64_t p_character_id) const {
	return characters.has(p_character_id);
}

int CharacterManager::get_character_count() const {
	return characters.size();
}

TypedArray<int> CharacterManager::get_all_character_ids() const {
	TypedArray<int> result;
	for (const KeyValue<uint64_t, GameCharacter *> &kv : characters) {
		result.push_back(kv.key);
	}
	return result;
}

TypedArray<int> CharacterManager::get_characters_by_type(int p_type) const {
	TypedArray<int> result;
	for (const KeyValue<uint64_t, GameCharacter *> &kv : characters) {
		if (kv.value->get_type() == static_cast<CharacterType>(p_type)) {
			result.push_back(kv.key);
		}
	}
	return result;
}

// ============================================
// === 基础属性读写 ===
// ============================================

String CharacterManager::get_character_name(uint64_t p_character_id) const {
	GameCharacter *c = get_character(p_character_id);
	ERR_FAIL_COND_V(!c, "");
	return c->get_name();
}

void CharacterManager::set_character_name(uint64_t p_character_id, const String &p_name) {
	GameCharacter *c = get_character(p_character_id);
	ERR_FAIL_COND(!c);
	c->set_name(p_name);
}

int CharacterManager::get_character_type(uint64_t p_character_id) const {
	GameCharacter *c = get_character(p_character_id);
	ERR_FAIL_COND_V(!c, -1);
	return static_cast<int>(c->get_type());
}

int CharacterManager::get_level(uint64_t p_character_id) const {
	GameCharacter *c = get_character(p_character_id);
	ERR_FAIL_COND_V(!c, 0);
	return c->get_level();
}

void CharacterManager::set_level(uint64_t p_character_id, int p_level) {
	GameCharacter *c = get_character(p_character_id);
	ERR_FAIL_COND(!c);
	c->set_level(p_level);
}

int CharacterManager::get_hp(uint64_t p_character_id) const {
	GameCharacter *c = get_character(p_character_id);
	ERR_FAIL_COND_V(!c, 0);
	return c->get_hp();
}

void CharacterManager::set_hp(uint64_t p_character_id, int p_hp) {
	GameCharacter *c = get_character(p_character_id);
	ERR_FAIL_COND(!c);
	c->set_hp(p_hp);
}

int CharacterManager::get_max_hp(uint64_t p_character_id) const {
	GameCharacter *c = get_character(p_character_id);
	ERR_FAIL_COND_V(!c, 0);
	return c->get_max_hp();
}

void CharacterManager::set_max_hp(uint64_t p_character_id, int p_max_hp) {
	GameCharacter *c = get_character(p_character_id);
	ERR_FAIL_COND(!c);
	c->set_max_hp(p_max_hp);
}

int CharacterManager::get_mp(uint64_t p_character_id) const {
	GameCharacter *c = get_character(p_character_id);
	ERR_FAIL_COND_V(!c, 0);
	return c->get_mp();
}

void CharacterManager::set_mp(uint64_t p_character_id, int p_mp) {
	GameCharacter *c = get_character(p_character_id);
	ERR_FAIL_COND(!c);
	c->set_mp(p_mp);
}

int CharacterManager::get_max_mp(uint64_t p_character_id) const {
	GameCharacter *c = get_character(p_character_id);
	ERR_FAIL_COND_V(!c, 0);
	return c->get_max_mp();
}

void CharacterManager::set_max_mp(uint64_t p_character_id, int p_max_mp) {
	GameCharacter *c = get_character(p_character_id);
	ERR_FAIL_COND(!c);
	c->set_max_mp(p_max_mp);
}

// ============================================
// === 战斗属性 ===
// ============================================

int CharacterManager::get_attack(uint64_t p_character_id) const {
	GameCharacter *c = get_character(p_character_id);
	ERR_FAIL_COND_V(!c, 0);
	return c->get_attack();
}

void CharacterManager::set_attack(uint64_t p_character_id, int p_attack) {
	GameCharacter *c = get_character(p_character_id);
	ERR_FAIL_COND(!c);
	c->set_attack(p_attack);
}

int CharacterManager::get_defense(uint64_t p_character_id) const {
	GameCharacter *c = get_character(p_character_id);
	ERR_FAIL_COND_V(!c, 0);
	return c->get_defense();
}

void CharacterManager::set_defense(uint64_t p_character_id, int p_defense) {
	GameCharacter *c = get_character(p_character_id);
	ERR_FAIL_COND(!c);
	c->set_defense(p_defense);
}

float CharacterManager::get_speed(uint64_t p_character_id) const {
	GameCharacter *c = get_character(p_character_id);
	ERR_FAIL_COND_V(!c, 0.0f);
	return c->get_speed();
}

void CharacterManager::set_speed(uint64_t p_character_id, float p_speed) {
	GameCharacter *c = get_character(p_character_id);
	ERR_FAIL_COND(!c);
	c->set_speed(p_speed);
}

// ============================================
// === 装备/背包 ===
// ============================================

uint64_t CharacterManager::get_inventory_id(uint64_t p_character_id) const {
	GameCharacter *c = get_character(p_character_id);
	ERR_FAIL_COND_V(!c, 0);
	return c->get_inventory_id();
}

void CharacterManager::set_inventory_id(uint64_t p_character_id, uint64_t p_inventory_id) {
	GameCharacter *c = get_character(p_character_id);
	ERR_FAIL_COND(!c);
	c->set_inventory_id(p_inventory_id);
}

uint64_t CharacterManager::get_equipment_id(uint64_t p_character_id) const {
	GameCharacter *c = get_character(p_character_id);
	ERR_FAIL_COND_V(!c, 0);
	return c->get_equipment_id();
}

void CharacterManager::set_equipment_id(uint64_t p_character_id, uint64_t p_equipment_id) {
	GameCharacter *c = get_character(p_character_id);
	ERR_FAIL_COND(!c);
	c->set_equipment_id(p_equipment_id);
}

// ============================================
// === 状态与战斗 ===
// ============================================

bool CharacterManager::is_alive(uint64_t p_character_id) const {
	GameCharacter *c = get_character(p_character_id);
	ERR_FAIL_COND_V(!c, false);
	return c->is_alive();
}

void CharacterManager::set_alive(uint64_t p_character_id, bool p_alive) {
	GameCharacter *c = get_character(p_character_id);
	ERR_FAIL_COND(!c);
	c->set_alive(p_alive);
}

int CharacterManager::take_damage(uint64_t p_character_id, int p_damage) {
	GameCharacter *c = get_character(p_character_id);
	ERR_FAIL_COND_V(!c, 0);
	return c->take_damage(p_damage);
}

void CharacterManager::heal(uint64_t p_character_id, int p_amount) {
	GameCharacter *c = get_character(p_character_id);
	ERR_FAIL_COND(!c);
	c->heal(p_amount);
}

// ============================================
// === 自定义属性 ===
// ============================================

void CharacterManager::set_custom_property(uint64_t p_character_id, const String &p_key, const Variant &p_value) {
	GameCharacter *c = get_character(p_character_id);
	ERR_FAIL_COND(!c);
	c->set_custom_property(p_key, p_value);
}

Variant CharacterManager::get_custom_property(uint64_t p_character_id, const String &p_key) const {
	GameCharacter *c = get_character(p_character_id);
	ERR_FAIL_COND_V(!c, Variant());
	return c->get_custom_property(p_key);
}

bool CharacterManager::has_custom_property(uint64_t p_character_id, const String &p_key) const {
	GameCharacter *c = get_character(p_character_id);
	ERR_FAIL_COND_V(!c, false);
	return c->has_custom_property(p_key);
}

void CharacterManager::remove_custom_property(uint64_t p_character_id, const String &p_key) {
	GameCharacter *c = get_character(p_character_id);
	ERR_FAIL_COND(!c);
	c->remove_custom_property(p_key);
}

// ============================================
// === 批量操作 ===
// ============================================

Dictionary CharacterManager::get_character_data(uint64_t p_character_id) const {
	Dictionary data;
	GameCharacter *c = get_character(p_character_id);
	ERR_FAIL_COND_V(!c, data);

	data["id"] = c->get_id();
	data["type"] = static_cast<int>(c->get_type());
	data["name"] = c->get_name();
	data["level"] = c->get_level();
	data["hp"] = c->get_hp();
	data["max_hp"] = c->get_max_hp();
	data["mp"] = c->get_mp();
	data["max_mp"] = c->get_max_mp();
	data["attack"] = c->get_attack();
	data["defense"] = c->get_defense();
	data["speed"] = c->get_speed();
	data["inventory_id"] = c->get_inventory_id();
	data["equipment_id"] = c->get_equipment_id();
	data["alive"] = c->is_alive();

	return data;
}

void CharacterManager::set_character_data(uint64_t p_character_id, const Dictionary &p_data) {
	GameCharacter *c = get_character(p_character_id);
	ERR_FAIL_COND(!c);

	if (p_data.has("name")) {
		c->set_name(p_data["name"]);
	}
	if (p_data.has("level")) {
		c->set_level(p_data["level"]);
	}
	if (p_data.has("max_hp")) {
		c->set_max_hp(p_data["max_hp"]);
	}
	if (p_data.has("hp")) {
		c->set_hp(p_data["hp"]);
	}
	if (p_data.has("max_mp")) {
		c->set_max_mp(p_data["max_mp"]);
	}
	if (p_data.has("mp")) {
		c->set_mp(p_data["mp"]);
	}
	if (p_data.has("attack")) {
		c->set_attack(p_data["attack"]);
	}
	if (p_data.has("defense")) {
		c->set_defense(p_data["defense"]);
	}
	if (p_data.has("speed")) {
		c->set_speed(p_data["speed"]);
	}
	if (p_data.has("inventory_id")) {
		c->set_inventory_id(p_data["inventory_id"]);
	}
	if (p_data.has("equipment_id")) {
		c->set_equipment_id(p_data["equipment_id"]);
	}
	if (p_data.has("alive")) {
		c->set_alive(p_data["alive"]);
	}
}

// ============================================
// === 序列化 ===
// ============================================

Dictionary CharacterManager::save_to_dict() const {
	Dictionary save_data;
	save_data["next_id"] = next_id;

	Array characters_array;
	for (const KeyValue<uint64_t, GameCharacter *> &kv : characters) {
		Dictionary char_data = get_character_data(kv.key);
		characters_array.push_back(char_data);
	}
	save_data["characters"] = characters_array;

	return save_data;
}

void CharacterManager::load_from_dict(const Dictionary &p_data) {
	clear_all_characters();

	if (p_data.has("next_id")) {
		next_id = p_data["next_id"];
	}

	if (!p_data.has("characters")) {
		return;
	}

	Array characters_array = p_data["characters"];

	for (int i = 0; i < characters_array.size(); i++) {
		Dictionary char_data = characters_array[i];
		uint64_t id = char_data["id"];
		int type = char_data["type"];

		create_character_with_id(id, type);
		set_character_data(id, char_data);
	}
}

// ============================================
// === 调试 ===
// ============================================

void CharacterManager::print_character(uint64_t p_character_id) const {
	GameCharacter *c = get_character(p_character_id);
	ERR_FAIL_COND(!c);
	print_line(c->to_string());
}

void CharacterManager::print_all_characters() const {
	print_line(vformat("=== CharacterManager: %d characters ===", characters.size()));
	for (const KeyValue<uint64_t, GameCharacter *> &kv : characters) {
		print_line(kv.value->to_string());
	}
}

void CharacterManager::clear_all_characters() {
	for (KeyValue<uint64_t, GameCharacter *> &kv : characters) {
		delete kv.value;
	}
	characters.clear();
	next_id = 1;
}

// ============================================
// === 内部辅助方法 ===
// ============================================

GameCharacter *CharacterManager::get_character(uint64_t p_character_id) const {
	if (characters.has(p_character_id)) {
		return characters[p_character_id];
	}
	return nullptr;
}
