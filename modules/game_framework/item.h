/**************************************************************************/
/*  item.h                                                                */
/**************************************************************************/

#pragma once

#include "core/object/gdvirtual.gen.inc"
#include "core/object/ref_counted.h"
#include "core/string/string_name.h"
#include "core/variant/dictionary.h"
#include "core/variant/typed_array.h"

// 物品稀有度
enum ItemRarity {
	ITEM_RARITY_COMMON,     // 普通
	ITEM_RARITY_UNCOMMON,   // 非凡
	ITEM_RARITY_RARE,       // 稀有
	ITEM_RARITY_EPIC,       // 史诗
	ITEM_RARITY_LEGENDARY,  // 传说
	ITEM_RARITY_MAX
};

// 物品类别（用于分类和UI显示）
enum ItemCategory {
	ITEM_CATEGORY_MATERIAL,    // 材料
	ITEM_CATEGORY_CONSUMABLE,  // 消耗品
	ITEM_CATEGORY_EQUIPMENT,   // 装备
	ITEM_CATEGORY_TOOL,        // 工具
	ITEM_CATEGORY_QUEST,       // 任务物品
	ITEM_CATEGORY_MISC,        // 杂项
	ITEM_CATEGORY_MAX
};

// 物品标签（位掩码，用于快速筛选）
enum ItemFlags : uint32_t {
	ITEM_FLAG_NONE        = 0,
	ITEM_FLAG_STACKABLE   = 1 << 0,  // 可堆叠
	ITEM_FLAG_TRADEABLE   = 1 << 1,  // 可交易
	ITEM_FLAG_DROPPABLE   = 1 << 2,  // 可丢弃
	ITEM_FLAG_CONSUMABLE  = 1 << 3,  // 可消耗
	ITEM_FLAG_EQUIPPABLE  = 1 << 4,  // 可装备
	ITEM_FLAG_USABLE      = 1 << 5,  // 可使用
	ITEM_FLAG_QUEST_ITEM  = 1 << 6,  // 任务物品（通常不可丢弃）
	ITEM_FLAG_UNIQUE      = 1 << 7,  // 唯一物品（只能持有一个）
};

// 物品基类 - 表示容器内的物品实例
class Item : public RefCounted {
	GDCLASS(Item, RefCounted);

public:
	// 物品最大堆叠数量上限
	static constexpr int32_t MAX_STACK_SIZE = 9999;
	// 无限耐久标记
	static constexpr int32_t DURABILITY_INFINITE = -1;

private:
	// === 核心标识 ===
	StringName item_id;              // 物品唯一ID（对应数据表）
	StringName display_name;         // 显示名称（可本地化）
	StringName description;          // 物品描述

	// === 堆叠与数量 ===
	int32_t quantity = 1;            // 当前数量
	int32_t max_stack_size = 1;      // 最大堆叠数量

	// === 耐久系统 ===
	int32_t durability = DURABILITY_INFINITE;      // 当前耐久（-1表示无限）
	int32_t max_durability = DURABILITY_INFINITE;  // 最大耐久

	// === 分类与标签 ===
	ItemCategory category = ITEM_CATEGORY_MISC;
	ItemRarity rarity = ITEM_RARITY_COMMON;
	uint32_t flags = ITEM_FLAG_STACKABLE | ITEM_FLAG_TRADEABLE | ITEM_FLAG_DROPPABLE;

	// === 经济属性 ===
	int32_t base_value = 0;          // 基础价值（用于买卖计算）
	float weight = 0.0f;             // 重量（用于负重系统）

	// === 扩展数据 ===
	Dictionary custom_data;          // 自定义数据（用于特殊物品属性）

protected:
	static void _bind_methods();

	// 虚函数（可被子类重写）
	GDVIRTUAL1R(bool, _on_use, Object *)       // 使用物品
	GDVIRTUAL1(_on_equip, Object *)            // 装备物品
	GDVIRTUAL1(_on_unequip, Object *)          // 卸下物品
	GDVIRTUAL0(_on_durability_depleted)        // 耐久耗尽

public:
	Item();
	virtual ~Item();

	// === 工厂方法 ===
	static Ref<Item> create(const StringName &p_item_id, int32_t p_quantity = 1);
	Ref<Item> duplicate() const;

	// === 核心标识访问器 ===
	void set_item_id(const StringName &p_id) { item_id = p_id; }
	StringName get_item_id() const { return item_id; }

	void set_display_name(const StringName &p_name) { display_name = p_name; }
	StringName get_display_name() const { return display_name; }

	void set_description(const StringName &p_desc) { description = p_desc; }
	StringName get_description() const { return description; }

	// === 堆叠系统 ===
	void set_quantity(int32_t p_quantity);
	int32_t get_quantity() const { return quantity; }

	void set_max_stack_size(int32_t p_max);
	int32_t get_max_stack_size() const { return max_stack_size; }

	int32_t get_available_stack_space() const;
	bool is_full_stack() const { return quantity >= max_stack_size; }
	bool is_empty() const { return quantity <= 0; }
	bool is_stackable() const { return has_flag(ITEM_FLAG_STACKABLE) && max_stack_size > 1; }

	// 堆叠操作
	int32_t add_quantity(int32_t p_amount);       // 返回溢出数量
	int32_t remove_quantity(int32_t p_amount);    // 返回实际移除数量
	bool can_stack_with(const Ref<Item> &p_other) const;
	int32_t stack_with(const Ref<Item> &p_other); // 将其他物品合并，返回剩余数量
	Ref<Item> split(int32_t p_amount);            // 分割堆叠

	// === 耐久系统 ===
	void set_durability(int32_t p_durability);
	int32_t get_durability() const { return durability; }

	void set_max_durability(int32_t p_max);
	int32_t get_max_durability() const { return max_durability; }

	bool has_durability() const { return max_durability != DURABILITY_INFINITE; }
	bool is_broken() const { return has_durability() && durability <= 0; }
	float get_durability_percent() const;

	void damage(int32_t p_amount = 1);   // 消耗耐久
	void repair(int32_t p_amount);       // 修复耐久
	void repair_full();                  // 完全修复

	// === 分类与标签 ===
	void set_category(ItemCategory p_category) { category = p_category; }
	ItemCategory get_category() const { return category; }

	void set_rarity(ItemRarity p_rarity) { rarity = p_rarity; }
	ItemRarity get_rarity() const { return rarity; }

	void set_flags(uint32_t p_flags) { flags = p_flags; }
	uint32_t get_flags() const { return flags; }

	void add_flag(ItemFlags p_flag) { flags |= p_flag; }
	void remove_flag(ItemFlags p_flag) { flags &= ~p_flag; }
	bool has_flag(ItemFlags p_flag) const { return (flags & p_flag) != 0; }

	// === 经济属性 ===
	void set_base_value(int32_t p_value) { base_value = p_value; }
	int32_t get_base_value() const { return base_value; }
	int32_t get_total_value() const { return base_value * quantity; }

	void set_weight(float p_weight) { weight = p_weight; }
	float get_weight() const { return weight; }
	float get_total_weight() const { return weight * quantity; }

	// === 扩展数据 ===
	void set_custom_data(const Dictionary &p_data) { custom_data = p_data; }
	Dictionary get_custom_data() const { return custom_data; }

	void set_custom_value(const StringName &p_key, const Variant &p_value);
	Variant get_custom_value(const StringName &p_key, const Variant &p_default = Variant()) const;
	bool has_custom_value(const StringName &p_key) const;
	void remove_custom_value(const StringName &p_key);

	// === 交互方法 ===
	bool use(Object *p_user);            // 使用物品
	void equip(Object *p_user);          // 装备物品
	void unequip(Object *p_user);        // 卸下物品

	// === 序列化 ===
	Dictionary serialize() const;
	void deserialize(const Dictionary &p_data);

	// === 辅助方法 ===
	String to_string() const;
	bool is_valid() const { return item_id != StringName() && quantity > 0; }
};

// 枚举类型转换宏
VARIANT_ENUM_CAST(ItemRarity);
VARIANT_ENUM_CAST(ItemCategory);
VARIANT_ENUM_CAST(ItemFlags);
