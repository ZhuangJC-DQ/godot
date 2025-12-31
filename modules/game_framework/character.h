/**************************************************************************/
/*  character.h                                                           */
/**************************************************************************/

#pragma once

#include "world_object.h"

#include "core/object/gdvirtual.gen.inc"
#include "core/string/string_name.h"

// 角色状态标志（位掩码）
enum CharacterState : uint32_t {
	CHARACTER_STATE_NONE       = 0,
	CHARACTER_STATE_ALIVE      = 1 << 0,   // 存活
	CHARACTER_STATE_MOVING     = 1 << 1,   // 移动中
	CHARACTER_STATE_ATTACKING  = 1 << 2,   // 攻击中
	CHARACTER_STATE_CASTING    = 1 << 3,   // 施法中
	CHARACTER_STATE_STUNNED    = 1 << 4,   // 眩晕
	CHARACTER_STATE_SILENCED   = 1 << 5,   // 沉默
	CHARACTER_STATE_ROOTED     = 1 << 6,   // 定身
	CHARACTER_STATE_INVINCIBLE = 1 << 7,   // 无敌
	CHARACTER_STATE_INVISIBLE  = 1 << 8,   // 隐身
};

// 伤害类型
enum DamageType {
	DAMAGE_TYPE_PHYSICAL,    // 物理伤害
	DAMAGE_TYPE_MAGICAL,     // 魔法伤害
	DAMAGE_TYPE_TRUE,        // 真实伤害（无视防御）
	DAMAGE_TYPE_PURE,        // 纯粹伤害（无视所有减免）
	DAMAGE_TYPE_MAX
};

// 角色基类 - 所有可控制/可交互的生物实体
class Character : public WorldObject {
	GDCLASS(Character, WorldObject);

public:
	// 角色阵营
	enum Faction {
		FACTION_NEUTRAL,    // 中立
		FACTION_FRIENDLY,   // 友方
		FACTION_HOSTILE,    // 敌对
		FACTION_MAX
	};

private:
	// === 基础标识 ===
	StringName character_name;       // 角色名称
	Faction faction = FACTION_NEUTRAL;

	// === 生命系统 ===
	float health = 100.0f;           // 当前生命值
	float max_health = 100.0f;       // 最大生命值
	float health_regen = 0.0f;       // 生命回复/秒

	// === 法力/能量系统 ===
	float mana = 100.0f;             // 当前法力值
	float max_mana = 100.0f;         // 最大法力值
	float mana_regen = 0.0f;         // 法力回复/秒

	// === 基础属性 ===
	float move_speed = 5.0f;         // 移动速度
	float attack_damage = 10.0f;     // 攻击力
	float attack_speed = 1.0f;       // 攻击速度
	float armor = 0.0f;              // 护甲（物理防御）
	float magic_resist = 0.0f;       // 魔抗（魔法防御）

	// === 状态系统 ===
	uint32_t state_flags = CHARACTER_STATE_ALIVE;

	// === 等级系统 ===
	int32_t level = 1;

protected:
	static void _bind_methods();

	// === 虚函数（子类重写）===
	GDVIRTUAL3(_on_take_damage, float, int, Object *)  // damage, type, source
	GDVIRTUAL2(_on_heal, float, Object *)              // amount, source
	GDVIRTUAL1(_on_death, Object *)                    // killer
	GDVIRTUAL0(_on_respawn)
	GDVIRTUAL1(_on_level_up, int32_t)                  // new_level

public:
	Character();
	virtual ~Character();

	// === 基础标识 ===
	void set_character_name(const StringName &p_name) { character_name = p_name; }
	StringName get_character_name() const { return character_name; }

	void set_faction(Faction p_faction) { faction = p_faction; }
	Faction get_faction() const { return faction; }

	// === 生命系统 ===
	void set_health(float p_health);
	float get_health() const { return health; }

	void set_max_health(float p_max);
	float get_max_health() const { return max_health; }

	void set_health_regen(float p_regen) { health_regen = p_regen; }
	float get_health_regen() const { return health_regen; }

	float get_health_percent() const;
	bool is_alive() const { return has_state(CHARACTER_STATE_ALIVE) && health > 0; }
	bool is_full_health() const { return health >= max_health; }

	// === 法力系统 ===
	void set_mana(float p_mana);
	float get_mana() const { return mana; }

	void set_max_mana(float p_max);
	float get_max_mana() const { return max_mana; }

	void set_mana_regen(float p_regen) { mana_regen = p_regen; }
	float get_mana_regen() const { return mana_regen; }

	float get_mana_percent() const;
	bool has_mana(float p_amount) const { return mana >= p_amount; }
	bool consume_mana(float p_amount);

	// === 基础属性 ===
	void set_move_speed(float p_speed) { move_speed = MAX(0.0f, p_speed); }
	float get_move_speed() const { return move_speed; }

	void set_attack_damage(float p_damage) { attack_damage = MAX(0.0f, p_damage); }
	float get_attack_damage() const { return attack_damage; }

	void set_attack_speed(float p_speed) { attack_speed = MAX(0.1f, p_speed); }
	float get_attack_speed() const { return attack_speed; }

	void set_armor(float p_armor) { armor = p_armor; }
	float get_armor() const { return armor; }

	void set_magic_resist(float p_resist) { magic_resist = p_resist; }
	float get_magic_resist() const { return magic_resist; }

	// === 状态系统 ===
	void set_state_flags(uint32_t p_flags) { state_flags = p_flags; }
	uint32_t get_state_flags() const { return state_flags; }

	void add_state(CharacterState p_state) { state_flags |= p_state; }
	void remove_state(CharacterState p_state) { state_flags &= ~p_state; }
	bool has_state(CharacterState p_state) const { return (state_flags & p_state) != 0; }
	void clear_states() { state_flags = CHARACTER_STATE_NONE; }

	// 便捷状态检查
	bool can_move() const;
	bool can_attack() const;
	bool can_cast() const;
	bool is_invincible() const { return has_state(CHARACTER_STATE_INVINCIBLE); }

	// === 等级系统 ===
	void set_level(int32_t p_level);
	int32_t get_level() const { return level; }

	// === 战斗系统 ===
	// 受到伤害，返回实际伤害值
	float take_damage(float p_amount, DamageType p_type = DAMAGE_TYPE_PHYSICAL, Object *p_source = nullptr);
	// 治疗，返回实际治疗量
	float heal(float p_amount, Object *p_source = nullptr);
	// 死亡
	void die(Object *p_killer = nullptr);
	// 复活
	void respawn(float p_health_percent = 1.0f);

	// 计算伤害减免后的伤害
	float calculate_damage_reduction(float p_damage, DamageType p_type) const;

	// === 时间更新（用于回复等）===
	virtual void tick(float p_delta);

	// === 序列化 ===
	Dictionary serialize() const override;
	void deserialize(const Dictionary &p_data) override;
};

VARIANT_ENUM_CAST(CharacterState);
VARIANT_ENUM_CAST(DamageType);
VARIANT_ENUM_CAST(Character::Faction);
