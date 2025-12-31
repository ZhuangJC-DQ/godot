/**************************************************************************/
/*  player.h                                                              */
/**************************************************************************/

#pragma once

#include "character.h"

// 玩家类 - 可被玩家控制的角色基类
class Player : public Character {
	GDCLASS(Player, Character);

private:
	// === 经验系统 ===
	int64_t experience = 0;                // 当前经验值
	int64_t experience_to_next_level = 100; // 升级所需经验

	// === 货币系统 ===
	int64_t gold = 0;                      // 金币

	// === 玩家标识 ===
	StringName player_id;                  // 玩家唯一ID（用于存档等）

	// === 统计数据 ===
	int32_t kills = 0;                     // 击杀数
	int32_t deaths = 0;                    // 死亡数
	float playtime = 0.0f;                 // 游戏时间（秒）

protected:
	static void _bind_methods();

	// === 虚函数（子类重写）===
	GDVIRTUAL1(_on_experience_gained, int64_t)  // amount
	GDVIRTUAL2(_on_gold_changed, int64_t, int64_t)  // old_amount, new_amount

public:
	Player();
	virtual ~Player();

	// === 经验系统 ===
	void set_experience(int64_t p_exp);
	int64_t get_experience() const { return experience; }

	void set_experience_to_next_level(int64_t p_exp) { experience_to_next_level = MAX(1LL, p_exp); }
	int64_t get_experience_to_next_level() const { return experience_to_next_level; }

	void add_experience(int64_t p_amount);
	float get_experience_percent() const;
	int64_t get_experience_needed() const { return experience_to_next_level - experience; }

	// === 货币系统 ===
	void set_gold(int64_t p_gold);
	int64_t get_gold() const { return gold; }

	bool has_gold(int64_t p_amount) const { return gold >= p_amount; }
	void add_gold(int64_t p_amount);
	bool spend_gold(int64_t p_amount);

	// === 玩家标识 ===
	void set_player_id(const StringName &p_id) { player_id = p_id; }
	StringName get_player_id() const { return player_id; }

	// === 统计数据 ===
	void set_kills(int32_t p_kills) { kills = MAX(0, p_kills); }
	int32_t get_kills() const { return kills; }
	void add_kill() { kills++; }

	void set_deaths(int32_t p_deaths) { deaths = MAX(0, p_deaths); }
	int32_t get_deaths() const { return deaths; }
	void add_death() { deaths++; }

	void set_playtime(float p_time) { playtime = MAX(0.0f, p_time); }
	float get_playtime() const { return playtime; }

	float get_kd_ratio() const { return deaths > 0 ? static_cast<float>(kills) / deaths : static_cast<float>(kills); }

	// === 时间更新 ===
	void tick(float p_delta) override;

	// === 升级计算（可重写）===
	virtual int64_t calculate_exp_for_level(int32_t p_level) const;
	virtual void on_level_up_stats();

	// === 序列化 ===
	Dictionary serialize() const override;
	void deserialize(const Dictionary &p_data) override;
};
