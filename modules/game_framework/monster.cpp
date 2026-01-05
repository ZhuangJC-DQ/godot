/**************************************************************************/
/*  monster.cpp                                                           */
/**************************************************************************/

#include "monster.h"

void Monster::_bind_methods() {

}

Monster::Monster() {
	// 怪物默认敌对阵营
	set_faction(FACTION_HOSTILE);

	// 怪物背包容量较小（用于掉落）
	init_container(4);
}

Monster::~Monster() = default;

// ============ 怪物属性 ============

void Monster::set_ai_state(MonsterAIState p_state) {
	if (ai_state == p_state) {
		return;
	}

	ai_state = p_state;

	// 状态变化时的处理
	if (ai_state == MONSTER_AI_DEAD) {
		clear_target();
	}
}

float Monster::get_rank_multiplier() const {
	switch (rank) {
		case MONSTER_RANK_NORMAL:
			return 1.0f;
		case MONSTER_RANK_ELITE:
			return 2.0f;
		case MONSTER_RANK_CHAMPION:
			return 3.0f;
		case MONSTER_RANK_BOSS:
			return 5.0f;
		case MONSTER_RANK_WORLD_BOSS:
			return 10.0f;
		default:
			return 1.0f;
	}
}

// ============ 仇恨系统 ============

void Monster::set_target(Object *p_target) {
	if (current_target == p_target) {
		return;
	}

	Object *old_target = current_target;
	current_target = p_target;

	if (old_target == nullptr && p_target != nullptr) {
		// 进入战斗
		GDVIRTUAL_CALL(_on_aggro, p_target);
	} else if (old_target != nullptr && p_target == nullptr) {
		// 脱离战斗
		GDVIRTUAL_CALL(_on_deaggro);
	}

	GDVIRTUAL_CALL(_on_target_changed, p_target);
}

void Monster::clear_target() {
	set_target(nullptr);
}

void Monster::enter_combat(Object *p_target) {
	if (p_target == nullptr || !is_alive()) {
		return;
	}

	set_target(p_target);
	set_ai_state(MONSTER_AI_CHASE);
}

void Monster::leave_combat() {
	clear_target();
	set_ai_state(MONSTER_AI_RETURN);
}

// ============ 奖励 ============

int64_t Monster::get_actual_exp_reward() const {
	return static_cast<int64_t>(exp_reward * get_rank_multiplier());
}

int64_t Monster::get_actual_gold_reward() const {
	return static_cast<int64_t>(gold_reward * get_rank_multiplier());
}

// ============ 时间更新 ============

void Monster::tick(float p_delta) {
	// 死亡状态处理
	if (!is_alive()) {
		if (can_respawn()) {
			death_timer += p_delta;
			if (death_timer >= respawn_time) {
				death_timer = 0.0f;
				respawn(1.0f);
				set_ai_state(MONSTER_AI_IDLE);
				set_local_position(spawn_position);
			}
		}
		return;
	}

	// 存活状态的基础更新（回复等）
	Character::tick(p_delta);
}

// ============ 序列化 ============

Dictionary Monster::serialize() const {
	Dictionary data = Character::serialize();

	// 怪物属性
	data["rank"] = static_cast<int>(rank);
	data["ai_state"] = static_cast<int>(ai_state);
	data["monster_id"] = monster_id;

	// AI 参数
	data["detection_range"] = detection_range;
	data["attack_range"] = attack_range;
	data["chase_range"] = chase_range;
	data["spawn_x"] = spawn_position.x;
	data["spawn_y"] = spawn_position.y;

	// 奖励
	data["exp_reward"] = exp_reward;
	data["gold_reward"] = gold_reward;
	data["loot_table_id"] = loot_table_id;

	// 重生
	data["respawn_time"] = respawn_time;

	return data;
}

void Monster::deserialize(const Dictionary &p_data) {
	Character::deserialize(p_data);

	// 怪物属性
	rank = static_cast<MonsterRank>((int)p_data.get("rank", MONSTER_RANK_NORMAL));
	ai_state = static_cast<MonsterAIState>((int)p_data.get("ai_state", MONSTER_AI_IDLE));
	monster_id = p_data.get("monster_id", StringName());

	// AI 参数
	detection_range = p_data.get("detection_range", 10.0f);
	attack_range = p_data.get("attack_range", 2.0f);
	chase_range = p_data.get("chase_range", 30.0f);
	spawn_position.x = p_data.get("spawn_x", 0);
	spawn_position.y = p_data.get("spawn_y", 0);

	// 奖励
	exp_reward = p_data.get("exp_reward", 10);
	gold_reward = p_data.get("gold_reward", 5);
	loot_table_id = p_data.get("loot_table_id", StringName());

	// 重生
	respawn_time = p_data.get("respawn_time", 60.0f);
}
