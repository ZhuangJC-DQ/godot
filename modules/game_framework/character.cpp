/**************************************************************************/
/*  character.cpp                                                         */
/**************************************************************************/

#include "character.h"

#include "core/math/math_funcs.h"

void Character::_bind_methods() {

}

Character::Character() {
	// 设置默认对象类型为角色类
	set_object_type(TYPE_INTERACTABLE);
}

Character::~Character() = default;

// ============ 生命系统 ============

void Character::set_health(float p_health) {
	health = CLAMP(p_health, 0.0f, max_health);
	if (health <= 0 && has_state(CHARACTER_STATE_ALIVE)) {
		die(nullptr);
	}
}

void Character::set_max_health(float p_max) {
	max_health = MAX(1.0f, p_max);
	if (health > max_health) {
		health = max_health;
	}
}

float Character::get_health_percent() const {
	if (max_health <= 0) {
		return 0.0f;
	}
	return health / max_health;
}

// ============ 法力系统 ============

void Character::set_mana(float p_mana) {
	mana = CLAMP(p_mana, 0.0f, max_mana);
}

void Character::set_max_mana(float p_max) {
	max_mana = MAX(0.0f, p_max);
	if (mana > max_mana) {
		mana = max_mana;
	}
}

float Character::get_mana_percent() const {
	if (max_mana <= 0) {
		return 0.0f;
	}
	return mana / max_mana;
}

bool Character::consume_mana(float p_amount) {
	if (p_amount <= 0) {
		return true;
	}
	if (mana < p_amount) {
		return false;
	}
	mana -= p_amount;
	return true;
}

// ============ 状态系统 ============

bool Character::can_move() const {
	if (!is_alive()) {
		return false;
	}
	if (has_state(CHARACTER_STATE_STUNNED) || has_state(CHARACTER_STATE_ROOTED)) {
		return false;
	}
	return true;
}

bool Character::can_attack() const {
	if (!is_alive()) {
		return false;
	}
	if (has_state(CHARACTER_STATE_STUNNED)) {
		return false;
	}
	return true;
}

bool Character::can_cast() const {
	if (!is_alive()) {
		return false;
	}
	if (has_state(CHARACTER_STATE_STUNNED) || has_state(CHARACTER_STATE_SILENCED)) {
		return false;
	}
	return true;
}

// ============ 等级系统 ============

void Character::set_level(int32_t p_level) {
	int32_t old_level = level;
	level = MAX(1, p_level);

	if (level > old_level) {
		GDVIRTUAL_CALL(_on_level_up, level);
	}
}

// ============ 战斗系统 ============

float Character::calculate_damage_reduction(float p_damage, DamageType p_type) const {
	if (p_damage <= 0) {
		return 0.0f;
	}

	switch (p_type) {
		case DAMAGE_TYPE_PHYSICAL: {
			// 护甲减伤公式：damage_reduction = armor / (armor + 100)
			// 100护甲 = 50%减伤，200护甲 = 66.7%减伤
			float reduction = armor / (armor + 100.0f);
			return p_damage * (1.0f - CLAMP(reduction, 0.0f, 0.9f)); // 最多90%减伤
		}
		case DAMAGE_TYPE_MAGICAL: {
			// 魔抗减伤，公式同上
			float reduction = magic_resist / (magic_resist + 100.0f);
			return p_damage * (1.0f - CLAMP(reduction, 0.0f, 0.9f));
		}
		case DAMAGE_TYPE_TRUE: {
			// 真实伤害无视护甲和魔抗
			return p_damage;
		}
		case DAMAGE_TYPE_PURE: {
			// 纯粹伤害无视所有减免
			return p_damage;
		}
		default:
			return p_damage;
	}
}

float Character::take_damage(float p_amount, DamageType p_type, Object *p_source) {
	if (p_amount <= 0 || !is_alive()) {
		return 0.0f;
	}

	// 无敌状态
	if (is_invincible() && p_type != DAMAGE_TYPE_PURE) {
		return 0.0f;
	}

	// 计算实际伤害
	float actual_damage = calculate_damage_reduction(p_amount, p_type);

	// 调用虚函数（允许子类修改伤害）
	GDVIRTUAL_CALL(_on_take_damage, actual_damage, static_cast<int>(p_type), p_source);

	// 扣血
	health = MAX(0.0f, health - actual_damage);

	// 检查死亡
	if (health <= 0) {
		die(p_source);
	}

	return actual_damage;
}

float Character::heal(float p_amount, Object *p_source) {
	if (p_amount <= 0 || !is_alive()) {
		return 0.0f;
	}

	float old_health = health;
	health = MIN(health + p_amount, max_health);
	float actual_heal = health - old_health;

	if (actual_heal > 0) {
		GDVIRTUAL_CALL(_on_heal, actual_heal, p_source);
	}

	return actual_heal;
}

void Character::die(Object *p_killer) {
	if (!has_state(CHARACTER_STATE_ALIVE)) {
		return; // 已经死亡
	}

	remove_state(CHARACTER_STATE_ALIVE);
	health = 0.0f;

	// 清除所有临时状态
	remove_state(CHARACTER_STATE_MOVING);
	remove_state(CHARACTER_STATE_ATTACKING);
	remove_state(CHARACTER_STATE_CASTING);

	GDVIRTUAL_CALL(_on_death, p_killer);
}

void Character::respawn(float p_health_percent) {
	if (has_state(CHARACTER_STATE_ALIVE)) {
		return; // 已经存活
	}

	add_state(CHARACTER_STATE_ALIVE);
	health = max_health * CLAMP(p_health_percent, 0.1f, 1.0f);
	mana = max_mana;

	GDVIRTUAL_CALL(_on_respawn);
}

// ============ 时间更新 ============

void Character::tick(float p_delta) {
	if (!is_alive()) {
		return;
	}

	// 生命回复
	if (health_regen > 0 && health < max_health) {
		health = MIN(health + health_regen * p_delta, max_health);
	}

	// 法力回复
	if (mana_regen > 0 && mana < max_mana) {
		mana = MIN(mana + mana_regen * p_delta, max_mana);
	}
}

// ============ 序列化 ============

Dictionary Character::serialize() const {
	Dictionary data = WorldObject::serialize();

	// 基础标识
	data["character_name"] = character_name;
	data["faction"] = static_cast<int>(faction);

	// 生命系统
	data["health"] = health;
	data["max_health"] = max_health;
	data["health_regen"] = health_regen;

	// 法力系统
	data["mana"] = mana;
	data["max_mana"] = max_mana;
	data["mana_regen"] = mana_regen;

	// 基础属性
	data["move_speed"] = move_speed;
	data["attack_damage"] = attack_damage;
	data["attack_speed"] = attack_speed;
	data["armor"] = armor;
	data["magic_resist"] = magic_resist;

	// 状态和等级
	data["state_flags"] = state_flags;
	data["level"] = level;

	return data;
}

void Character::deserialize(const Dictionary &p_data) {
	WorldObject::deserialize(p_data);

	// 基础标识
	character_name = p_data.get("character_name", StringName());
	faction = static_cast<Faction>((int)p_data.get("faction", FACTION_NEUTRAL));

	// 生命系统
	max_health = p_data.get("max_health", 100.0f);
	health = p_data.get("health", max_health);
	health_regen = p_data.get("health_regen", 0.0f);

	// 法力系统
	max_mana = p_data.get("max_mana", 100.0f);
	mana = p_data.get("mana", max_mana);
	mana_regen = p_data.get("mana_regen", 0.0f);

	// 基础属性
	move_speed = p_data.get("move_speed", 5.0f);
	attack_damage = p_data.get("attack_damage", 10.0f);
	attack_speed = p_data.get("attack_speed", 1.0f);
	armor = p_data.get("armor", 0.0f);
	magic_resist = p_data.get("magic_resist", 0.0f);

	// 状态和等级
	state_flags = p_data.get("state_flags", CHARACTER_STATE_ALIVE);
	level = p_data.get("level", 1);
}
