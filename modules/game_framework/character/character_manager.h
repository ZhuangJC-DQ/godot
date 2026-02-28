/**************************************************************************/
/*  character_manager.h                                                   */
/**************************************************************************/

#ifndef CHARACTER_MANAGER_H
#define CHARACTER_MANAGER_H

#include "game_character.h"

#include "core/object/object.h"
#include "core/templates/hash_map.h"
#include "core/variant/typed_array.h"

class CharacterManager : public Object {
	GDCLASS(CharacterManager, Object);

private:
	static CharacterManager *singleton;

	HashMap<uint64_t, GameCharacter *> characters;
	uint64_t next_id = 1;

protected:
	static void _bind_methods();

public:
	static CharacterManager *get_singleton();

	CharacterManager();
	~CharacterManager();

	// ============================================
	// === 角色生命周期 ===
	// ============================================

	// 创建角色（type: 0=Player, 1=NPC, 2=Enemy）
	uint64_t create_character(int p_type);

	// 服务器指定 ID 创建角色（联机模式预留接口）
	uint64_t create_character_with_id(uint64_t p_id, int p_type);

	// 删除角色
	void destroy_character(uint64_t p_character_id);

	// 检查角色是否有效
	bool is_valid_character(uint64_t p_character_id) const;

	// 获取所有角色数量
	int get_character_count() const;

	// 获取所有角色 ID
	TypedArray<int> get_all_character_ids() const;

	// 按类型获取角色 ID
	TypedArray<int> get_characters_by_type(int p_type) const;

	// ============================================
	// === 基础属性读写 ===
	// ============================================

	String get_character_name(uint64_t p_character_id) const;
	void set_character_name(uint64_t p_character_id, const String &p_name);

	int get_character_type(uint64_t p_character_id) const;

	int get_level(uint64_t p_character_id) const;
	void set_level(uint64_t p_character_id, int p_level);

	int get_hp(uint64_t p_character_id) const;
	void set_hp(uint64_t p_character_id, int p_hp);

	int get_max_hp(uint64_t p_character_id) const;
	void set_max_hp(uint64_t p_character_id, int p_max_hp);

	int get_mp(uint64_t p_character_id) const;
	void set_mp(uint64_t p_character_id, int p_mp);

	int get_max_mp(uint64_t p_character_id) const;
	void set_max_mp(uint64_t p_character_id, int p_max_mp);

	// ============================================
	// === 战斗属性 ===
	// ============================================

	int get_attack(uint64_t p_character_id) const;
	void set_attack(uint64_t p_character_id, int p_attack);

	int get_defense(uint64_t p_character_id) const;
	void set_defense(uint64_t p_character_id, int p_defense);

	float get_speed(uint64_t p_character_id) const;
	void set_speed(uint64_t p_character_id, float p_speed);

	// ============================================
	// === 装备/背包（关联 ItemManager）===
	// ============================================

	uint64_t get_inventory_id(uint64_t p_character_id) const;
	void set_inventory_id(uint64_t p_character_id, uint64_t p_inventory_id);

	uint64_t get_equipment_id(uint64_t p_character_id) const;
	void set_equipment_id(uint64_t p_character_id, uint64_t p_equipment_id);

	// ============================================
	// === 状态与战斗 ===
	// ============================================

	bool is_alive(uint64_t p_character_id) const;
	void set_alive(uint64_t p_character_id, bool p_alive);

	// 受到伤害，返回实际伤害值
	int take_damage(uint64_t p_character_id, int p_damage);

	// 治疗
	void heal(uint64_t p_character_id, int p_amount);

	// ============================================
	// === 自定义属性 ===
	// ============================================

	void set_custom_property(uint64_t p_character_id, const String &p_key, const Variant &p_value);
	Variant get_custom_property(uint64_t p_character_id, const String &p_key) const;
	bool has_custom_property(uint64_t p_character_id, const String &p_key) const;
	void remove_custom_property(uint64_t p_character_id, const String &p_key);

	// ============================================
	// === 批量操作 ===
	// ============================================

	Dictionary get_character_data(uint64_t p_character_id) const;
	void set_character_data(uint64_t p_character_id, const Dictionary &p_data);

	// ============================================
	// === 序列化 ===
	// ============================================

	Dictionary save_to_dict() const;
	void load_from_dict(const Dictionary &p_data);

	// ============================================
	// === 调试 ===
	// ============================================

	void print_character(uint64_t p_character_id) const;
	void print_all_characters() const;
	void clear_all_characters();

private:
	GameCharacter *get_character(uint64_t p_character_id) const;
};

#endif // CHARACTER_MANAGER_H
