/**************************************************************************/
/*  monster.h                                                             */
/**************************************************************************/

#pragma once

#include "character.h"

// 怪物等级类型
enum MonsterRank {
	MONSTER_RANK_NORMAL,      // 普通怪
	MONSTER_RANK_ELITE,       // 精英怪
	MONSTER_RANK_CHAMPION,    // 冠军怪
	MONSTER_RANK_BOSS,        // BOSS
	MONSTER_RANK_WORLD_BOSS,  // 世界BOSS
	MONSTER_RANK_MAX
};

// 怪物AI状态
enum MonsterAIState {
	MONSTER_AI_IDLE,          // 空闲
	MONSTER_AI_PATROL,        // 巡逻
	MONSTER_AI_CHASE,         // 追击
	MONSTER_AI_ATTACK,        // 攻击
	MONSTER_AI_RETURN,        // 返回
	MONSTER_AI_DEAD,          // 死亡
	MONSTER_AI_MAX
};

// 怪物类 - 敌对生物
class Monster : public Character {
	GDCLASS(Monster, Character);

private:
	// === 怪物属性 ===
	MonsterRank rank = MONSTER_RANK_NORMAL;
	MonsterAIState ai_state = MONSTER_AI_IDLE;
	StringName monster_id;            // 怪物模板ID

	// === AI 参数 ===
	float detection_range = 10.0f;    // 侦测范围
	float attack_range = 2.0f;        // 攻击范围
	float chase_range = 30.0f;        // 追击范围（超出返回）
	Vector2i spawn_position;          // 出生点

	// === 仇恨系统 ===
	Object *current_target = nullptr; // 当前目标

	// === 奖励 ===
	int64_t exp_reward = 10;          // 经验奖励
	int64_t gold_reward = 5;          // 金币奖励
	StringName loot_table_id;         // 掉落表ID

	// === 重生 ===
	float respawn_time = 60.0f;       // 重生时间（秒）
	float death_timer = 0.0f;         // 死亡计时器

protected:
	static void _bind_methods();

	// === 虚函数 ===
	GDVIRTUAL1(_on_aggro, Object *)       // 进入战斗
	GDVIRTUAL0(_on_deaggro)               // 脱离战斗
	GDVIRTUAL1(_on_target_changed, Object *)  // 目标改变

public:
	Monster();
	virtual ~Monster();

	// === 怪物属性 ===
	void set_rank(MonsterRank p_rank) { rank = p_rank; }
	MonsterRank get_rank() const { return rank; }

	void set_ai_state(MonsterAIState p_state);
	MonsterAIState get_ai_state() const { return ai_state; }

	void set_monster_id(const StringName &p_id) { monster_id = p_id; }
	StringName get_monster_id() const { return monster_id; }

	// 等级倍率（精英/BOSS等有额外属性）
	float get_rank_multiplier() const;

	// === AI 参数 ===
	void set_detection_range(float p_range) { detection_range = MAX(0.0f, p_range); }
	float get_detection_range() const { return detection_range; }

	void set_attack_range(float p_range) { attack_range = MAX(0.0f, p_range); }
	float get_attack_range() const { return attack_range; }

	void set_chase_range(float p_range) { chase_range = MAX(0.0f, p_range); }
	float get_chase_range() const { return chase_range; }

	void set_spawn_position(const Vector2i &p_pos) { spawn_position = p_pos; }
	Vector2i get_spawn_position() const { return spawn_position; }

	// === 仇恨系统 ===
	void set_target(Object *p_target);
	Object *get_target() const { return current_target; }
	bool has_target() const { return current_target != nullptr; }
	void clear_target();

	// 进入/脱离战斗
	void enter_combat(Object *p_target);
	void leave_combat();
	bool is_in_combat() const { return current_target != nullptr; }

	// === 奖励 ===
	void set_exp_reward(int64_t p_exp) { exp_reward = MAX(0LL, p_exp); }
	int64_t get_exp_reward() const { return exp_reward; }

	void set_gold_reward(int64_t p_gold) { gold_reward = MAX(0LL, p_gold); }
	int64_t get_gold_reward() const { return gold_reward; }

	void set_loot_table_id(const StringName &p_id) { loot_table_id = p_id; }
	StringName get_loot_table_id() const { return loot_table_id; }

	// 获取实际奖励（考虑等级倍率）
	int64_t get_actual_exp_reward() const;
	int64_t get_actual_gold_reward() const;

	// === 重生 ===
	void set_respawn_time(float p_time) { respawn_time = MAX(0.0f, p_time); }
	float get_respawn_time() const { return respawn_time; }

	bool can_respawn() const { return respawn_time > 0; }

	// === 时间更新 ===
	void tick(float p_delta) override;

	// === 序列化 ===
	Dictionary serialize() const override;
	void deserialize(const Dictionary &p_data) override;
};

VARIANT_ENUM_CAST(MonsterRank);
VARIANT_ENUM_CAST(MonsterAIState);
