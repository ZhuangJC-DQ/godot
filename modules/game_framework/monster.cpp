/**************************************************************************/
/*  monster.cpp                                                           */
/**************************************************************************/

#include "monster.h"

void Monster::_bind_methods() {
	// === 枚举绑定 ===
	// MonsterRank
	BIND_ENUM_CONSTANT(MONSTER_RANK_NORMAL);
	BIND_ENUM_CONSTANT(MONSTER_RANK_ELITE);
	BIND_ENUM_CONSTANT(MONSTER_RANK_CHAMPION);
	BIND_ENUM_CONSTANT(MONSTER_RANK_BOSS);
	BIND_ENUM_CONSTANT(MONSTER_RANK_WORLD_BOSS);

	// MonsterAIState
	BIND_ENUM_CONSTANT(MONSTER_AI_IDLE);
	BIND_ENUM_CONSTANT(MONSTER_AI_PATROL);
	BIND_ENUM_CONSTANT(MONSTER_AI_CHASE);
	BIND_ENUM_CONSTANT(MONSTER_AI_ATTACK);
	BIND_ENUM_CONSTANT(MONSTER_AI_RETURN);
	BIND_ENUM_CONSTANT(MONSTER_AI_DEAD);

	// === 怪物属性 ===
	ClassDB::bind_method(D_METHOD("set_rank", "rank"), &Monster::set_rank);
	ClassDB::bind_method(D_METHOD("get_rank"), &Monster::get_rank);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "rank", PROPERTY_HINT_ENUM, "Normal,Elite,Champion,Boss,WorldBoss"), "set_rank", "get_rank");

	ClassDB::bind_method(D_METHOD("set_ai_state", "state"), &Monster::set_ai_state);
	ClassDB::bind_method(D_METHOD("get_ai_state"), &Monster::get_ai_state);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "ai_state", PROPERTY_HINT_ENUM, "Idle,Patrol,Chase,Attack,Return,Dead"), "set_ai_state", "get_ai_state");

	ClassDB::bind_method(D_METHOD("set_monster_id", "id"), &Monster::set_monster_id);
	ClassDB::bind_method(D_METHOD("get_monster_id"), &Monster::get_monster_id);
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "monster_id"), "set_monster_id", "get_monster_id");

	ClassDB::bind_method(D_METHOD("get_rank_multiplier"), &Monster::get_rank_multiplier);

	// === AI 参数 ===
	ClassDB::bind_method(D_METHOD("set_detection_range", "range"), &Monster::set_detection_range);
	ClassDB::bind_method(D_METHOD("get_detection_range"), &Monster::get_detection_range);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "detection_range"), "set_detection_range", "get_detection_range");

	ClassDB::bind_method(D_METHOD("set_attack_range", "range"), &Monster::set_attack_range);
	ClassDB::bind_method(D_METHOD("get_attack_range"), &Monster::get_attack_range);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "attack_range"), "set_attack_range", "get_attack_range");

	ClassDB::bind_method(D_METHOD("set_chase_range", "range"), &Monster::set_chase_range);
	ClassDB::bind_method(D_METHOD("get_chase_range"), &Monster::get_chase_range);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "chase_range"), "set_chase_range", "get_chase_range");

	ClassDB::bind_method(D_METHOD("set_spawn_position", "position"), &Monster::set_spawn_position);
	ClassDB::bind_method(D_METHOD("get_spawn_position"), &Monster::get_spawn_position);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2I, "spawn_position"), "set_spawn_position", "get_spawn_position");

	// === 仇恨系统 ===
	ClassDB::bind_method(D_METHOD("set_target", "target"), &Monster::set_target);
	ClassDB::bind_method(D_METHOD("get_target"), &Monster::get_target);
	ClassDB::bind_method(D_METHOD("has_target"), &Monster::has_target);
	ClassDB::bind_method(D_METHOD("clear_target"), &Monster::clear_target);

	ClassDB::bind_method(D_METHOD("enter_combat", "target"), &Monster::enter_combat);
	ClassDB::bind_method(D_METHOD("leave_combat"), &Monster::leave_combat);
	ClassDB::bind_method(D_METHOD("is_in_combat"), &Monster::is_in_combat);

	// === 奖励 ===
	ClassDB::bind_method(D_METHOD("set_exp_reward", "exp"), &Monster::set_exp_reward);
	ClassDB::bind_method(D_METHOD("get_exp_reward"), &Monster::get_exp_reward);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "exp_reward"), "set_exp_reward", "get_exp_reward");

	ClassDB::bind_method(D_METHOD("set_gold_reward", "gold"), &Monster::set_gold_reward);
	ClassDB::bind_method(D_METHOD("get_gold_reward"), &Monster::get_gold_reward);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "gold_reward"), "set_gold_reward", "get_gold_reward");

	ClassDB::bind_method(D_METHOD("set_loot_table_id", "id"), &Monster::set_loot_table_id);
	ClassDB::bind_method(D_METHOD("get_loot_table_id"), &Monster::get_loot_table_id);
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "loot_table_id"), "set_loot_table_id", "get_loot_table_id");

	ClassDB::bind_method(D_METHOD("get_actual_exp_reward"), &Monster::get_actual_exp_reward);
	ClassDB::bind_method(D_METHOD("get_actual_gold_reward"), &Monster::get_actual_gold_reward);

	// === 重生 ===
	ClassDB::bind_method(D_METHOD("set_respawn_time", "time"), &Monster::set_respawn_time);
	ClassDB::bind_method(D_METHOD("get_respawn_time"), &Monster::get_respawn_time);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "respawn_time"), "set_respawn_time", "get_respawn_time");

	ClassDB::bind_method(D_METHOD("can_respawn"), &Monster::can_respawn);

	// === 虚函数绑定 ===
	GDVIRTUAL_BIND(_on_aggro, "target");
	GDVIRTUAL_BIND(_on_deaggro);
	GDVIRTUAL_BIND(_on_target_changed, "new_target");
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
