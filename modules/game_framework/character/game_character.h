/**************************************************************************/
/*  game_character.h                                                      */
/**************************************************************************/

#ifndef GAME_CHARACTER_H
#define GAME_CHARACTER_H

#include "core/string/ustring.h"
#include "core/templates/hash_map.h"
#include "core/variant/variant.h"

// 角色类型枚举
enum CharacterType {
	CHARACTER_TYPE_PLAYER = 0,
	CHARACTER_TYPE_NPC = 1,
	CHARACTER_TYPE_ENEMY = 2,
	CHARACTER_TYPE_MAX
};

// 纯数据类，不暴露给 GDScript
class GameCharacter {
private:
	uint64_t id = 0;
	CharacterType type = CHARACTER_TYPE_PLAYER;
	String name;

	// === 基础属性 ===
	int level = 1;
	int hp = 100;
	int max_hp = 100;
	int mp = 50;
	int max_mp = 50;

	// === 战斗属性 ===
	int attack = 10;
	int defense = 5;
	float speed = 5.0f;

	// === 装备/背包容器 ID（关联 ItemManager）===
	uint64_t inventory_id = 0; // 背包容器 ID
	uint64_t equipment_id = 0; // 装备容器 ID

	// === 状态 ===
	bool alive = true;

	// === 自定义属性 ===
	HashMap<String, Variant> custom_properties;

public:
	GameCharacter();
	GameCharacter(uint64_t p_id, CharacterType p_type);
	~GameCharacter();

	// === ID 和类型 ===
	uint64_t get_id() const { return id; }
	void set_id(uint64_t p_id) { id = p_id; }

	CharacterType get_type() const { return type; }
	void set_type(CharacterType p_type) { type = p_type; }

	// === 基础属性 ===
	String get_name() const { return name; }
	void set_name(const String &p_name) { name = p_name; }

	int get_level() const { return level; }
	void set_level(int p_level) { level = p_level; }

	int get_hp() const { return hp; }
	void set_hp(int p_hp);

	int get_max_hp() const { return max_hp; }
	void set_max_hp(int p_max_hp);

	int get_mp() const { return mp; }
	void set_mp(int p_mp);

	int get_max_mp() const { return max_mp; }
	void set_max_mp(int p_max_mp);

	// === 战斗属性 ===
	int get_attack() const { return attack; }
	void set_attack(int p_attack) { attack = p_attack; }

	int get_defense() const { return defense; }
	void set_defense(int p_defense) { defense = p_defense; }

	float get_speed() const { return speed; }
	void set_speed(float p_speed) { speed = p_speed; }

	// === 装备/背包 ===
	uint64_t get_inventory_id() const { return inventory_id; }
	void set_inventory_id(uint64_t p_id) { inventory_id = p_id; }

	uint64_t get_equipment_id() const { return equipment_id; }
	void set_equipment_id(uint64_t p_id) { equipment_id = p_id; }

	// === 状态 ===
	bool is_alive() const { return alive; }
	void set_alive(bool p_alive) { alive = p_alive; }

	// === 战斗计算 ===
	int take_damage(int p_damage);
	void heal(int p_amount);

	// === 自定义属性 ===
	void set_custom_property(const String &p_key, const Variant &p_value);
	Variant get_custom_property(const String &p_key) const;
	bool has_custom_property(const String &p_key) const;
	void remove_custom_property(const String &p_key);

	// === 调试 ===
	String to_string() const;
};

#endif // GAME_CHARACTER_H
