/**************************************************************************/
/*  npc.cpp                                                               */
/**************************************************************************/

#include "npc.h"

void NPC::_bind_methods() {
	// === 枚举绑定 ===
	// NPCBehavior
	BIND_ENUM_CONSTANT(NPC_BEHAVIOR_IDLE);
	BIND_ENUM_CONSTANT(NPC_BEHAVIOR_PATROL);
	BIND_ENUM_CONSTANT(NPC_BEHAVIOR_WANDER);
	BIND_ENUM_CONSTANT(NPC_BEHAVIOR_FOLLOW);
	BIND_ENUM_CONSTANT(NPC_BEHAVIOR_FLEE);

	// NPCType
	BIND_ENUM_CONSTANT(NPC_TYPE_VILLAGER);
	BIND_ENUM_CONSTANT(NPC_TYPE_MERCHANT);
	BIND_ENUM_CONSTANT(NPC_TYPE_QUEST_GIVER);
	BIND_ENUM_CONSTANT(NPC_TYPE_TRAINER);
	BIND_ENUM_CONSTANT(NPC_TYPE_GUARD);

	// === NPC 类型 ===
	ClassDB::bind_method(D_METHOD("set_npc_type", "type"), &NPC::set_npc_type);
	ClassDB::bind_method(D_METHOD("get_npc_type"), &NPC::get_npc_type);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "npc_type", PROPERTY_HINT_ENUM, "Villager,Merchant,QuestGiver,Trainer,Guard"), "set_npc_type", "get_npc_type");

	ClassDB::bind_method(D_METHOD("set_behavior", "behavior"), &NPC::set_behavior);
	ClassDB::bind_method(D_METHOD("get_behavior"), &NPC::get_behavior);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "behavior", PROPERTY_HINT_ENUM, "Idle,Patrol,Wander,Follow,Flee"), "set_behavior", "get_behavior");

	// === 对话系统 ===
	ClassDB::bind_method(D_METHOD("set_dialogue_id", "id"), &NPC::set_dialogue_id);
	ClassDB::bind_method(D_METHOD("get_dialogue_id"), &NPC::get_dialogue_id);
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "dialogue_id"), "set_dialogue_id", "get_dialogue_id");

	ClassDB::bind_method(D_METHOD("set_can_talk", "can_talk"), &NPC::set_can_talk);
	ClassDB::bind_method(D_METHOD("get_can_talk"), &NPC::get_can_talk);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "can_talk"), "set_can_talk", "get_can_talk");

	ClassDB::bind_method(D_METHOD("talk_to", "player"), &NPC::talk_to);

	// === 商店系统 ===
	ClassDB::bind_method(D_METHOD("set_is_merchant", "is_merchant"), &NPC::set_is_merchant);
	ClassDB::bind_method(D_METHOD("get_is_merchant"), &NPC::get_is_merchant);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "is_merchant"), "set_is_merchant", "get_is_merchant");

	ClassDB::bind_method(D_METHOD("set_shop_id", "id"), &NPC::set_shop_id);
	ClassDB::bind_method(D_METHOD("get_shop_id"), &NPC::get_shop_id);
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "shop_id"), "set_shop_id", "get_shop_id");

	// === AI 参数 ===
	ClassDB::bind_method(D_METHOD("set_aggro_range", "range"), &NPC::set_aggro_range);
	ClassDB::bind_method(D_METHOD("get_aggro_range"), &NPC::get_aggro_range);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "aggro_range"), "set_aggro_range", "get_aggro_range");

	ClassDB::bind_method(D_METHOD("set_leash_range", "range"), &NPC::set_leash_range);
	ClassDB::bind_method(D_METHOD("get_leash_range"), &NPC::get_leash_range);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "leash_range"), "set_leash_range", "get_leash_range");

	ClassDB::bind_method(D_METHOD("set_spawn_position", "position"), &NPC::set_spawn_position);
	ClassDB::bind_method(D_METHOD("get_spawn_position"), &NPC::get_spawn_position);
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2I, "spawn_position"), "set_spawn_position", "get_spawn_position");

	ClassDB::bind_method(D_METHOD("is_aggressive"), &NPC::is_aggressive);

	// === 掉落表 ===
	ClassDB::bind_method(D_METHOD("set_loot_table_id", "id"), &NPC::set_loot_table_id);
	ClassDB::bind_method(D_METHOD("get_loot_table_id"), &NPC::get_loot_table_id);
	ADD_PROPERTY(PropertyInfo(Variant::STRING_NAME, "loot_table_id"), "set_loot_table_id", "get_loot_table_id");

	// === 交互 ===
	ClassDB::bind_method(D_METHOD("can_interact_with", "actor"), &NPC::can_interact_with);

	// === 虚函数绑定 ===
	GDVIRTUAL_BIND(_on_talk_to, "player");
	GDVIRTUAL_BIND(_on_can_interact, "actor");
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
