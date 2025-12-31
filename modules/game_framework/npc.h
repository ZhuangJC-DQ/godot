/**************************************************************************/
/*  npc.h                                                                 */
/**************************************************************************/

#pragma once

#include "character.h"

// NPC 行为类型
enum NPCBehavior {
	NPC_BEHAVIOR_IDLE,        // 静止
	NPC_BEHAVIOR_PATROL,      // 巡逻
	NPC_BEHAVIOR_WANDER,      // 游荡
	NPC_BEHAVIOR_FOLLOW,      // 跟随
	NPC_BEHAVIOR_FLEE,        // 逃跑
	NPC_BEHAVIOR_MAX
};

// NPC 类型
enum NPCType {
	NPC_TYPE_VILLAGER,        // 村民
	NPC_TYPE_MERCHANT,        // 商人
	NPC_TYPE_QUEST_GIVER,     // 任务NPC
	NPC_TYPE_TRAINER,         // 训练师
	NPC_TYPE_GUARD,           // 守卫
	NPC_TYPE_MAX
};

// NPC 基类 - 非玩家角色
class NPC : public Character {
	GDCLASS(NPC, Character);

private:
	// === NPC 类型 ===
	NPCType npc_type = NPC_TYPE_VILLAGER;
	NPCBehavior behavior = NPC_BEHAVIOR_IDLE;

	// === 对话系统 ===
	StringName dialogue_id;           // 对话ID（指向对话数据）
	bool can_talk = true;             // 是否可以对话

	// === 商店系统 ===
	bool is_merchant = false;         // 是否是商人
	StringName shop_id;               // 商店ID（指向商品列表）

	// === AI 参数 ===
	float aggro_range = 0.0f;         // 仇恨范围（0表示不主动攻击）
	float leash_range = 50.0f;        // 拴绳范围（超出返回原点）
	Vector2i spawn_position;          // 出生点

	// === 掉落表 ===
	StringName loot_table_id;         // 掉落表ID

protected:
	static void _bind_methods();

	// === 虚函数 ===
	GDVIRTUAL1(_on_talk_to, Object *)          // 与玩家对话
	GDVIRTUAL1R(bool, _on_can_interact, Object *)  // 是否可以交互

public:
	NPC();
	virtual ~NPC();

	// === NPC 类型 ===
	void set_npc_type(NPCType p_type) { npc_type = p_type; }
	NPCType get_npc_type() const { return npc_type; }

	void set_behavior(NPCBehavior p_behavior) { behavior = p_behavior; }
	NPCBehavior get_behavior() const { return behavior; }

	// === 对话系统 ===
	void set_dialogue_id(const StringName &p_id) { dialogue_id = p_id; }
	StringName get_dialogue_id() const { return dialogue_id; }

	void set_can_talk(bool p_can) { can_talk = p_can; }
	bool get_can_talk() const { return can_talk; }

	void talk_to(Object *p_player);

	// === 商店系统 ===
	void set_is_merchant(bool p_is) { is_merchant = p_is; }
	bool get_is_merchant() const { return is_merchant; }

	void set_shop_id(const StringName &p_id) { shop_id = p_id; }
	StringName get_shop_id() const { return shop_id; }

	// === AI 参数 ===
	void set_aggro_range(float p_range) { aggro_range = MAX(0.0f, p_range); }
	float get_aggro_range() const { return aggro_range; }

	void set_leash_range(float p_range) { leash_range = MAX(0.0f, p_range); }
	float get_leash_range() const { return leash_range; }

	void set_spawn_position(const Vector2i &p_pos) { spawn_position = p_pos; }
	Vector2i get_spawn_position() const { return spawn_position; }

	bool is_aggressive() const { return aggro_range > 0; }

	// === 掉落表 ===
	void set_loot_table_id(const StringName &p_id) { loot_table_id = p_id; }
	StringName get_loot_table_id() const { return loot_table_id; }

	// === 交互 ===
	bool can_interact_with(Object *p_actor);

	// === 序列化 ===
	Dictionary serialize() const override;
	void deserialize(const Dictionary &p_data) override;
};

VARIANT_ENUM_CAST(NPCBehavior);
VARIANT_ENUM_CAST(NPCType);
