/**************************************************************************/
/*  npc.cpp                                                               */
/**************************************************************************/

#include "npc.h"

void NPC::_bind_methods() {

}

NPC::NPC() {
	// NPC 默认中立阵营
	set_faction(FACTION_NEUTRAL);

	// NPC 背包容量较小
	init_container(6);
}

NPC::~NPC() = default;

// ============ 对话系统 ============

void NPC::talk_to(Object *p_player) {
	if (!can_talk || !is_alive()) {
		return;
	}

	GDVIRTUAL_CALL(_on_talk_to, p_player);
}

// ============ 交互 ============

bool NPC::can_interact_with(Object *p_actor) {
	if (!is_alive()) {
		return false;
	}

	bool result = true;
	if (GDVIRTUAL_CALL(_on_can_interact, p_actor, result)) {
		return result;
	}

	return can_talk || is_merchant;
}

// ============ 序列化 ============

Dictionary NPC::serialize() const {
	Dictionary data = Character::serialize();

	// NPC 类型
	data["npc_type"] = static_cast<int>(npc_type);
	data["behavior"] = static_cast<int>(behavior);

	// 对话系统
	data["dialogue_id"] = dialogue_id;
	data["can_talk"] = can_talk;

	// 商店系统
	data["is_merchant"] = is_merchant;
	data["shop_id"] = shop_id;

	// AI 参数
	data["aggro_range"] = aggro_range;
	data["leash_range"] = leash_range;
	data["spawn_x"] = spawn_position.x;
	data["spawn_y"] = spawn_position.y;

	// 掉落表
	data["loot_table_id"] = loot_table_id;

	return data;
}

void NPC::deserialize(const Dictionary &p_data) {
	Character::deserialize(p_data);

	// NPC 类型
	npc_type = static_cast<NPCType>((int)p_data.get("npc_type", NPC_TYPE_VILLAGER));
	behavior = static_cast<NPCBehavior>((int)p_data.get("behavior", NPC_BEHAVIOR_IDLE));

	// 对话系统
	dialogue_id = p_data.get("dialogue_id", StringName());
	can_talk = p_data.get("can_talk", true);

	// 商店系统
	is_merchant = p_data.get("is_merchant", false);
	shop_id = p_data.get("shop_id", StringName());

	// AI 参数
	aggro_range = p_data.get("aggro_range", 0.0f);
	leash_range = p_data.get("leash_range", 50.0f);
	spawn_position.x = p_data.get("spawn_x", 0);
	spawn_position.y = p_data.get("spawn_y", 0);

	// 掉落表
	loot_table_id = p_data.get("loot_table_id", StringName());
}
