/**************************************************************************/
/*  world_item.cpp                                                        */
/**************************************************************************/

#include "world_item.h"

void WorldItem::_bind_methods() {
	// 枚举绑定
	BIND_ENUM_CONSTANT(TYPE_GENERIC);
	BIND_ENUM_CONSTANT(TYPE_CONTAINER);
	BIND_ENUM_CONSTANT(TYPE_RESOURCE);
	BIND_ENUM_CONSTANT(TYPE_FURNITURE);
	BIND_ENUM_CONSTANT(TYPE_MAX);

	// 属性绑定
	ClassDB::bind_method(D_METHOD("get_item_id"), &WorldItem::get_item_id);
	ClassDB::bind_method(D_METHOD("get_item_type"), &WorldItem::get_item_type);
	ClassDB::bind_method(D_METHOD("get_local_position"), &WorldItem::get_local_position);
	ClassDB::bind_method(D_METHOD("set_local_position", "position"), &WorldItem::set_local_position);

	// 容器方法绑定
	ClassDB::bind_method(D_METHOD("has_container"), &WorldItem::has_container);
	ClassDB::bind_method(D_METHOD("get_container_capacity"), &WorldItem::get_container_capacity);
	ClassDB::bind_method(D_METHOD("get_container_used_slots"), &WorldItem::get_container_used_slots);
	ClassDB::bind_method(D_METHOD("container_clear"), &WorldItem::container_clear);

	// 交互方法绑定
	ClassDB::bind_method(D_METHOD("interact", "actor"), &WorldItem::interact);
	ClassDB::bind_method(D_METHOD("harvest", "actor"), &WorldItem::harvest);

	// 序列化绑定
	ClassDB::bind_method(D_METHOD("serialize"), &WorldItem::serialize);
	ClassDB::bind_method(D_METHOD("deserialize", "data"), &WorldItem::deserialize);

	// 虚函数绑定（允许GDScript重写）
	GDVIRTUAL_BIND(_on_interact, "actor");
	GDVIRTUAL_BIND(_on_harvest, "actor", "loot");
}

WorldItem::WorldItem() = default;

WorldItem::~WorldItem() = default;

void WorldItem::setup(const StringName &p_item_id, ItemType p_type, const Vector2i &p_position) {
	item_id = p_item_id;
	item_type = p_type;
	local_position = p_position;
}

// ============ 容器系统 ============

void WorldItem::init_container(int32_t p_capacity) {
	ERR_FAIL_COND_MSG(p_capacity < 0, "Container capacity cannot be negative.");
	container_capacity = p_capacity;
	container.resize(p_capacity);
}

int32_t WorldItem::get_container_used_slots() const {
	int32_t count = 0;
	for (const ItemStack &stack : container) {
		if (!stack.is_empty()) {
			count++;
		}
	}
	return count;
}

bool WorldItem::container_add(const StringName &p_item_id, int32_t p_quantity, const Dictionary &p_metadata) {
	ERR_FAIL_COND_V_MSG(p_quantity <= 0, false, "Quantity must be positive.");
	ERR_FAIL_COND_V_MSG(!has_container(), false, "Item has no container.");

	// 查找空槽位
	for (int32_t i = 0; i < container_capacity; i++) {
		if (container[i].is_empty()) {
			container.write[i].item_id = p_item_id;
			container.write[i].quantity = p_quantity;
			container.write[i].metadata = p_metadata;
			return true;
		}
	}
	return false; // 容器已满
}

ItemStack WorldItem::container_remove(int32_t p_slot, int32_t p_quantity) {
	ERR_FAIL_INDEX_V(p_slot, container_capacity, ItemStack());

	ItemStack &stack = container.write[p_slot];
	if (stack.is_empty()) {
		return ItemStack();
	}

	ItemStack result;
	if (p_quantity < 0 || p_quantity >= stack.quantity) {
		// 移除全部
		result = stack;
		stack.clear();
	} else {
		// 部分移除
		result.item_id = stack.item_id;
		result.quantity = p_quantity;
		result.metadata = stack.metadata;
		stack.quantity -= p_quantity;
	}
	return result;
}

ItemStack WorldItem::container_get(int32_t p_slot) const {
	ERR_FAIL_INDEX_V(p_slot, container_capacity, ItemStack());
	return container[p_slot];
}

void WorldItem::container_clear() {
	for (ItemStack &stack : container) {
		stack.clear();
	}
}

// ============ 交互接口 ============

void WorldItem::interact(Object *p_actor) {
	GDVIRTUAL_CALL(_on_interact, p_actor);
}

TypedArray<Dictionary> WorldItem::harvest(Object *p_actor) {
	TypedArray<Dictionary> loot;

	// 默认行为：将容器内容作为掉落物
	for (const ItemStack &stack : container) {
		if (!stack.is_empty()) {
			Dictionary loot_item;
			loot_item["item_id"] = stack.item_id;
			loot_item["quantity"] = stack.quantity;
			loot_item["metadata"] = stack.metadata;
			loot.push_back(loot_item);
		}
	}

	// 调用虚函数让子类可以修改掉落物
	GDVIRTUAL_CALL(_on_harvest, p_actor, loot);

	return loot;
}

// ============ 序列化 ============

Dictionary WorldItem::serialize() const {
	Dictionary data;
	data["item_id"] = item_id;
	data["item_type"] = static_cast<int>(item_type);
	data["position_x"] = local_position.x;
	data["position_y"] = local_position.y;

	if (has_container()) {
		data["container_capacity"] = container_capacity;
		Array container_data;
		for (int32_t i = 0; i < container.size(); i++) {
			const ItemStack &stack = container[i];
			if (!stack.is_empty()) {
				Dictionary stack_data;
				stack_data["slot"] = i;
				stack_data["item_id"] = stack.item_id;
				stack_data["quantity"] = stack.quantity;
				stack_data["metadata"] = stack.metadata;
				container_data.push_back(stack_data);
			}
		}
		if (!container_data.is_empty()) {
			data["container"] = container_data;
		}
	}

	return data;
}

void WorldItem::deserialize(const Dictionary &p_data) {
	item_id = p_data.get("item_id", StringName());
	item_type = static_cast<ItemType>((int)p_data.get("item_type", TYPE_GENERIC));
	local_position.x = p_data.get("position_x", 0);
	local_position.y = p_data.get("position_y", 0);

	if (p_data.has("container_capacity")) {
		init_container(p_data["container_capacity"]);

		if (p_data.has("container")) {
			Array container_data = p_data["container"];
			for (int i = 0; i < container_data.size(); i++) {
				Dictionary stack_data = container_data[i];
				int slot = stack_data.get("slot", -1);
				if (slot >= 0 && slot < container_capacity) {
					container.write[slot].item_id = stack_data.get("item_id", StringName());
					container.write[slot].quantity = stack_data.get("quantity", 0);
					container.write[slot].metadata = stack_data.get("metadata", Dictionary());
				}
			}
		}
	}
}
