/**************************************************************************/
/*  player.cpp                                                            */
/**************************************************************************/

#include "player.h"

void Player::_bind_methods() {

}

Player::Player() {
	// 玩家默认友方阵营
	set_faction(FACTION_FRIENDLY);

	// 初始化默认背包容量
	init_container(20);
}

Player::~Player() = default;

// ============ 经验系统 ============

void Player::set_experience(int64_t p_exp) {
	experience = MAX(0LL, p_exp);
}

void Player::add_experience(int64_t p_amount) {
	if (p_amount <= 0) {
		return;
	}

	experience += p_amount;
	GDVIRTUAL_CALL(_on_experience_gained, p_amount);

	// 检查升级
	while (experience >= experience_to_next_level) {
		experience -= experience_to_next_level;
		set_level(get_level() + 1);
		on_level_up_stats();
		experience_to_next_level = calculate_exp_for_level(get_level() + 1);
	}
}

float Player::get_experience_percent() const {
	if (experience_to_next_level <= 0) {
		return 1.0f;
	}
	return static_cast<float>(experience) / static_cast<float>(experience_to_next_level);
}

int64_t Player::calculate_exp_for_level(int32_t p_level) const {
	// 默认升级经验公式：base * (level ^ 1.5)
	// 可在子类中重写
	return static_cast<int64_t>(100.0 * Math::pow(static_cast<double>(p_level), 1.5));
}

void Player::on_level_up_stats() {
	// 默认升级属性加成，可在子类中重写
	set_max_health(get_max_health() + 10.0f);
	set_health(get_max_health()); // 升级回满血

	set_max_mana(get_max_mana() + 5.0f);
	set_mana(get_max_mana()); // 升级回满蓝

	set_attack_damage(get_attack_damage() + 2.0f);
}

// ============ 货币系统 ============

void Player::set_gold(int64_t p_gold) {
	int64_t old_gold = gold;
	gold = MAX(0LL, p_gold);
	if (gold != old_gold) {
		GDVIRTUAL_CALL(_on_gold_changed, old_gold, gold);
	}
}

void Player::add_gold(int64_t p_amount) {
	if (p_amount == 0) {
		return;
	}
	set_gold(gold + p_amount);
}

bool Player::spend_gold(int64_t p_amount) {
	if (p_amount <= 0) {
		return true;
	}
	if (gold < p_amount) {
		return false;
	}
	set_gold(gold - p_amount);
	return true;
}

// ============ 时间更新 ============

void Player::tick(float p_delta) {
	Character::tick(p_delta);

	// 累计游戏时间
	playtime += p_delta;
}

// ============ 序列化 ============

Dictionary Player::serialize() const {
	Dictionary data = Character::serialize();

	// 经验系统
	data["experience"] = experience;
	data["experience_to_next_level"] = experience_to_next_level;

	// 货币系统
	data["gold"] = gold;

	// 玩家标识
	data["player_id"] = player_id;

	// 统计数据
	data["kills"] = kills;
	data["deaths"] = deaths;
	data["playtime"] = playtime;

	return data;
}

void Player::deserialize(const Dictionary &p_data) {
	Character::deserialize(p_data);

	// 经验系统
	experience = p_data.get("experience", 0);
	experience_to_next_level = p_data.get("experience_to_next_level", 100);

	// 货币系统
	gold = p_data.get("gold", 0);

	// 玩家标识
	player_id = p_data.get("player_id", StringName());

	// 统计数据
	kills = p_data.get("kills", 0);
	deaths = p_data.get("deaths", 0);
	playtime = p_data.get("playtime", 0.0f);
}
