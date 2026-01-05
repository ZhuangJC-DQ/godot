/**************************************************************************/
/*  item.cpp                                                              */
/**************************************************************************/

#include "item.h"

#include "core/string/print_string.h"
#include "core/variant/variant.h"

void Item::_bind_methods() {

}

Item::Item() = default;

Item::~Item() = default;

// ============ 工厂方法 ============

Ref<Item> Item::create(const StringName &p_item_id, int32_t p_quantity) {
	Ref<Item> item;
	item.instantiate();
	item->set_item_id(p_item_id);
	item->set_quantity(p_quantity);
	return item;
}

Ref<Item> Item::duplicate() const {
	Ref<Item> copy;
	copy.instantiate();

	copy->item_id = item_id;
	copy->display_name = display_name;
	copy->description = description;
	copy->quantity = quantity;
	copy->max_stack_size = max_stack_size;
	copy->durability = durability;
	copy->max_durability = max_durability;
	copy->category = category;
	copy->rarity = rarity;
	copy->flags = flags;
	copy->base_value = base_value;
	copy->weight = weight;
	copy->custom_data = custom_data.duplicate(true);

	return copy;
}

// ============ 堆叠系统 ============

void Item::set_quantity(int32_t p_quantity) {
	quantity = CLAMP(p_quantity, 0, max_stack_size);
}

void Item::set_max_stack_size(int32_t p_max) {
	max_stack_size = CLAMP(p_max, 1, MAX_STACK_SIZE);
	if (quantity > max_stack_size) {
		quantity = max_stack_size;
	}
}

int32_t Item::get_available_stack_space() const {
	return max_stack_size - quantity;
}

int32_t Item::add_quantity(int32_t p_amount) {
	ERR_FAIL_COND_V_MSG(p_amount < 0, 0, "Cannot add negative quantity.");

	int32_t space = get_available_stack_space();
	if (p_amount <= space) {
		quantity += p_amount;
		return 0;
	} else {
		quantity = max_stack_size;
		return p_amount - space;
	}
}

int32_t Item::remove_quantity(int32_t p_amount) {
	ERR_FAIL_COND_V_MSG(p_amount < 0, 0, "Cannot remove negative quantity.");

	int32_t removed = MIN(p_amount, quantity);
	quantity -= removed;
	return removed;
}

bool Item::can_stack_with(const Ref<Item> &p_other) const {
	if (p_other.is_null()) {
		return false;
	}
	if (!is_stackable() || !p_other->is_stackable()) {
		return false;
	}
	if (item_id != p_other->item_id) {
		return false;
	}
	if (is_full_stack()) {
		return false;
	}
	// 有耐久的物品通常不能堆叠（除非耐久相同）
	if (has_durability() && (durability != p_other->durability)) {
		return false;
	}
	return true;
}

int32_t Item::stack_with(const Ref<Item> &p_other) {
	ERR_FAIL_COND_V_MSG(p_other.is_null(), -1, "Cannot stack with null item.");
	ERR_FAIL_COND_V_MSG(!can_stack_with(p_other), -1, "Items cannot be stacked together.");

	int32_t other_qty = p_other->get_quantity();
	int32_t overflow = add_quantity(other_qty);
	p_other->set_quantity(overflow);

	return overflow;
}

Ref<Item> Item::split(int32_t p_amount) {
	ERR_FAIL_COND_V_MSG(p_amount <= 0, Ref<Item>(), "Split amount must be positive.");
	ERR_FAIL_COND_V_MSG(p_amount >= quantity, Ref<Item>(), "Cannot split all or more items.");

	Ref<Item> split_item = duplicate();
	split_item->set_quantity(p_amount);
	quantity -= p_amount;

	return split_item;
}

// ============ 耐久系统 ============

void Item::set_durability(int32_t p_durability) {
	if (p_durability == DURABILITY_INFINITE) {
		durability = DURABILITY_INFINITE;
	} else {
		durability = CLAMP(p_durability, 0, max_durability);
	}
}

void Item::set_max_durability(int32_t p_max) {
	max_durability = p_max;
	if (max_durability != DURABILITY_INFINITE && durability > max_durability) {
		durability = max_durability;
	}
}

float Item::get_durability_percent() const {
	if (!has_durability() || max_durability <= 0) {
		return 1.0f;
	}
	return static_cast<float>(durability) / static_cast<float>(max_durability);
}

void Item::damage(int32_t p_amount) {
	if (!has_durability()) {
		return;
	}
	ERR_FAIL_COND_MSG(p_amount < 0, "Damage amount cannot be negative.");

	durability = MAX(0, durability - p_amount);

	if (durability <= 0) {
		GDVIRTUAL_CALL(_on_durability_depleted);
	}
}

void Item::repair(int32_t p_amount) {
	if (!has_durability()) {
		return;
	}
	ERR_FAIL_COND_MSG(p_amount < 0, "Repair amount cannot be negative.");

	durability = MIN(max_durability, durability + p_amount);
}

void Item::repair_full() {
	if (has_durability()) {
		durability = max_durability;
	}
}

// ============ 扩展数据 ============

void Item::set_custom_value(const StringName &p_key, const Variant &p_value) {
	custom_data[p_key] = p_value;
}

Variant Item::get_custom_value(const StringName &p_key, const Variant &p_default) const {
	if (custom_data.has(p_key)) {
		return custom_data[p_key];
	}
	return p_default;
}

bool Item::has_custom_value(const StringName &p_key) const {
	return custom_data.has(p_key);
}

void Item::remove_custom_value(const StringName &p_key) {
	custom_data.erase(p_key);
}

// ============ 交互方法 ============

bool Item::use(Object *p_user) {
	if (!has_flag(ITEM_FLAG_USABLE)) {
		return false;
	}

	bool result = false;
	if (GDVIRTUAL_CALL(_on_use, p_user, result)) {
		if (result && has_flag(ITEM_FLAG_CONSUMABLE)) {
			remove_quantity(1);
		}
		return result;
	}
	return false;
}

void Item::equip(Object *p_user) {
	if (!has_flag(ITEM_FLAG_EQUIPPABLE)) {
		return;
	}
	GDVIRTUAL_CALL(_on_equip, p_user);
}

void Item::unequip(Object *p_user) {
	GDVIRTUAL_CALL(_on_unequip, p_user);
}

// ============ 序列化 ============

Dictionary Item::serialize() const {
	Dictionary data;

	// 核心数据
	data["item_id"] = item_id;
	data["quantity"] = quantity;

	// 只保存非默认值以节省空间
	if (display_name != StringName()) {
		data["display_name"] = display_name;
	}
	if (description != StringName()) {
		data["description"] = description;
	}
	if (max_stack_size != 1) {
		data["max_stack_size"] = max_stack_size;
	}
	if (durability != DURABILITY_INFINITE) {
		data["durability"] = durability;
	}
	if (max_durability != DURABILITY_INFINITE) {
		data["max_durability"] = max_durability;
	}
	if (category != ITEM_CATEGORY_MISC) {
		data["category"] = static_cast<int>(category);
	}
	if (rarity != ITEM_RARITY_COMMON) {
		data["rarity"] = static_cast<int>(rarity);
	}
	if (flags != (ITEM_FLAG_STACKABLE | ITEM_FLAG_TRADEABLE | ITEM_FLAG_DROPPABLE)) {
		data["flags"] = flags;
	}
	if (base_value != 0) {
		data["base_value"] = base_value;
	}
	if (weight != 0.0f) {
		data["weight"] = weight;
	}
	if (!custom_data.is_empty()) {
		data["custom_data"] = custom_data;
	}

	return data;
}

void Item::deserialize(const Dictionary &p_data) {
	item_id = p_data.get("item_id", StringName());
	quantity = p_data.get("quantity", 1);
	display_name = p_data.get("display_name", StringName());
	description = p_data.get("description", StringName());
	max_stack_size = p_data.get("max_stack_size", 1);
	durability = p_data.get("durability", DURABILITY_INFINITE);
	max_durability = p_data.get("max_durability", DURABILITY_INFINITE);
	category = static_cast<ItemCategory>((int)p_data.get("category", static_cast<int>(ITEM_CATEGORY_MISC)));
	rarity = static_cast<ItemRarity>((int)p_data.get("rarity", static_cast<int>(ITEM_RARITY_COMMON)));
	flags = p_data.get("flags", ITEM_FLAG_STACKABLE | ITEM_FLAG_TRADEABLE | ITEM_FLAG_DROPPABLE);
	base_value = p_data.get("base_value", 0);
	weight = p_data.get("weight", 0.0f);
	custom_data = p_data.get("custom_data", Dictionary());
}

// ============ 辅助方法 ============

String Item::to_string() const {
	return vformat("Item[%s x%d]", item_id, quantity);
}
