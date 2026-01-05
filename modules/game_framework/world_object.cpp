/**************************************************************************/
/*  world_object.cpp                                                      */
/**************************************************************************/

#include "world_object.h"

void WorldObject::_bind_methods() {

}

WorldObject::WorldObject() = default;

WorldObject::~WorldObject() = default;

void WorldObject::setup(const StringName &p_object_id, ObjectType p_type, const Vector2i &p_position) {
	object_id = p_object_id;
	object_type = p_type;
	local_position = p_position;
}

// ============ 容器系统 ============

void WorldObject::init_container(int32_t p_capacity) {
	ERR_FAIL_COND_MSG(p_capacity < 0, "Container capacity cannot be negative.");
	container_capacity = p_capacity;
	container.resize(p_capacity);
	for (int32_t i = 0; i < p_capacity; i++) {
		container.write[i] = Ref<Item>();
	}
}

int32_t WorldObject::get_container_used_slots() const {
	int32_t count = 0;
	for (const Ref<Item> &item : container) {
		if (item.is_valid() && !item->is_empty()) {
			count++;
		}
	}
	return count;
}

int32_t WorldObject::get_container_empty_slots() const {
	return container_capacity - get_container_used_slots();
}

bool WorldObject::is_container_full() const {
	return get_container_used_slots() >= container_capacity;
}

bool WorldObject::is_container_empty() const {
	return get_container_used_slots() == 0;
}

bool WorldObject::container_add_item(const Ref<Item> &p_item) {
	ERR_FAIL_COND_V_MSG(p_item.is_null(), false, "Cannot add null item.");
	ERR_FAIL_COND_V_MSG(!has_container(), false, "Object has no container.");

	// 先尝试堆叠到现有物品
	if (p_item->is_stackable() && container_try_stack(p_item)) {
		if (p_item->is_empty()) {
			return true;
		}
	}

	// 查找空槽位
	for (int32_t i = 0; i < container_capacity; i++) {
		if (container[i].is_null() || container[i]->is_empty()) {
			container.write[i] = p_item;
			GDVIRTUAL_CALL(_on_item_added, i, p_item);
			return true;
		}
	}

	return false; // 容器已满
}

bool WorldObject::container_add_item_at(int32_t p_slot, const Ref<Item> &p_item) {
	ERR_FAIL_INDEX_V(p_slot, container_capacity, false);
	ERR_FAIL_COND_V_MSG(p_item.is_null(), false, "Cannot add null item.");

	// 检查槽位是否为空
	if (container[p_slot].is_valid() && !container[p_slot]->is_empty()) {
		// 尝试堆叠
		if (container[p_slot]->can_stack_with(p_item)) {
			container[p_slot]->stack_with(p_item);
			return p_item->is_empty();
		}
		return false;
	}

	container.write[p_slot] = p_item;
	GDVIRTUAL_CALL(_on_item_added, p_slot, p_item);
	return true;
}

Ref<Item> WorldObject::container_remove_item(int32_t p_slot) {
	ERR_FAIL_INDEX_V(p_slot, container_capacity, Ref<Item>());

	Ref<Item> item = container[p_slot];
	if (item.is_valid()) {
		container.write[p_slot] = Ref<Item>();
		GDVIRTUAL_CALL(_on_item_removed, p_slot, item);
	}
	return item;
}

Ref<Item> WorldObject::container_get_item(int32_t p_slot) const {
	ERR_FAIL_INDEX_V(p_slot, container_capacity, Ref<Item>());
	return container[p_slot];
}

bool WorldObject::container_set_item(int32_t p_slot, const Ref<Item> &p_item) {
	ERR_FAIL_INDEX_V(p_slot, container_capacity, false);

	Ref<Item> old_item = container[p_slot];
	if (old_item.is_valid()) {
		GDVIRTUAL_CALL(_on_item_removed, p_slot, old_item);
	}

	container.write[p_slot] = p_item;

	if (p_item.is_valid()) {
		GDVIRTUAL_CALL(_on_item_added, p_slot, p_item);
	}

	return true;
}

void WorldObject::container_clear() {
	for (int32_t i = 0; i < container.size(); i++) {
		if (container[i].is_valid()) {
			GDVIRTUAL_CALL(_on_item_removed, i, container[i]);
			container.write[i] = Ref<Item>();
		}
	}
}

int32_t WorldObject::container_find_item(const StringName &p_item_id) const {
	for (int32_t i = 0; i < container.size(); i++) {
		if (container[i].is_valid() && container[i]->get_item_id() == p_item_id) {
			return i;
		}
	}
	return -1;
}

int32_t WorldObject::container_count_item(const StringName &p_item_id) const {
	int32_t count = 0;
	for (const Ref<Item> &item : container) {
		if (item.is_valid() && item->get_item_id() == p_item_id) {
			count += item->get_quantity();
		}
	}
	return count;
}

bool WorldObject::container_has_item(const StringName &p_item_id, int32_t p_quantity) const {
	return container_count_item(p_item_id) >= p_quantity;
}

int32_t WorldObject::container_add_items(const StringName &p_item_id, int32_t p_quantity) {
	ERR_FAIL_COND_V_MSG(p_quantity <= 0, 0, "Quantity must be positive.");
	ERR_FAIL_COND_V_MSG(!has_container(), p_quantity, "Object has no container.");

	int32_t remaining = p_quantity;

	// 先尝试添加到现有堆叠
	for (int32_t i = 0; i < container.size() && remaining > 0; i++) {
		if (container[i].is_valid() && container[i]->get_item_id() == p_item_id && !container[i]->is_full_stack()) {
			remaining = container[i]->add_quantity(remaining);
		}
	}

	// 创建新堆叠
	while (remaining > 0) {
		int32_t slot = -1;
		for (int32_t i = 0; i < container_capacity; i++) {
			if (container[i].is_null() || container[i]->is_empty()) {
				slot = i;
				break;
			}
		}

		if (slot < 0) {
			break; // 没有空槽位
		}

		Ref<Item> new_item = Item::create(p_item_id, remaining);
		remaining = 0; // 默认全部添加（如果有堆叠限制会被调整）
		container.write[slot] = new_item;
		GDVIRTUAL_CALL(_on_item_added, slot, new_item);
	}

	return remaining;
}

int32_t WorldObject::container_remove_items(const StringName &p_item_id, int32_t p_quantity) {
	ERR_FAIL_COND_V_MSG(p_quantity <= 0, 0, "Quantity must be positive.");

	int32_t removed = 0;
	int32_t to_remove = p_quantity;

	for (int32_t i = 0; i < container.size() && to_remove > 0; i++) {
		if (container[i].is_valid() && container[i]->get_item_id() == p_item_id) {
			int32_t qty = container[i]->get_quantity();
			if (qty <= to_remove) {
				removed += qty;
				to_remove -= qty;
				GDVIRTUAL_CALL(_on_item_removed, i, container[i]);
				container.write[i] = Ref<Item>();
			} else {
				container[i]->remove_quantity(to_remove);
				removed += to_remove;
				to_remove = 0;
			}
		}
	}

	return removed;
}

TypedArray<Ref<Item>> WorldObject::container_get_all_items() const {
	TypedArray<Ref<Item>> items;
	for (const Ref<Item> &item : container) {
		if (item.is_valid() && !item->is_empty()) {
			items.push_back(item);
		}
	}
	return items;
}

bool WorldObject::container_try_stack(const Ref<Item> &p_item) {
	ERR_FAIL_COND_V_MSG(p_item.is_null(), false, "Cannot stack null item.");

	if (!p_item->is_stackable()) {
		return false;
	}

	bool stacked = false;
	for (int32_t i = 0; i < container.size() && !p_item->is_empty(); i++) {
		if (container[i].is_valid() && container[i]->can_stack_with(p_item)) {
			container[i]->stack_with(p_item);
			stacked = true;
		}
	}

	return stacked;
}

// ============ 交互接口 ============

void WorldObject::interact(Object *p_actor) {
	GDVIRTUAL_CALL(_on_interact, p_actor);
}

TypedArray<Dictionary> WorldObject::harvest(Object *p_actor) {
	TypedArray<Dictionary> loot;

	// 默认行为：将容器内容作为掉落物
	for (const Ref<Item> &item : container) {
		if (item.is_valid() && !item->is_empty()) {
			loot.push_back(item->serialize());
		}
	}

	// 调用虚函数让子类可以修改掉落物
	GDVIRTUAL_CALL(_on_harvest, p_actor, loot);

	return loot;
}

// ============ 序列化 ============

Dictionary WorldObject::serialize() const {
	Dictionary data;
	data["object_id"] = object_id;
	data["object_type"] = static_cast<int>(object_type);
	data["position_x"] = local_position.x;
	data["position_y"] = local_position.y;

	if (has_container()) {
		data["container_capacity"] = container_capacity;
		Array container_data;
		for (int32_t i = 0; i < container.size(); i++) {
			const Ref<Item> &item = container[i];
			if (item.is_valid() && !item->is_empty()) {
				Dictionary slot_data;
				slot_data["slot"] = i;
				slot_data["item"] = item->serialize();
				container_data.push_back(slot_data);
			}
		}
		if (!container_data.is_empty()) {
			data["container"] = container_data;
		}
	}

	return data;
}

void WorldObject::deserialize(const Dictionary &p_data) {
	object_id = p_data.get("object_id", StringName());
	object_type = static_cast<ObjectType>((int)p_data.get("object_type", TYPE_GENERIC));
	local_position.x = p_data.get("position_x", 0);
	local_position.y = p_data.get("position_y", 0);

	if (p_data.has("container_capacity")) {
		init_container(p_data["container_capacity"]);

		if (p_data.has("container")) {
			Array container_data = p_data["container"];
			for (int i = 0; i < container_data.size(); i++) {
				Dictionary slot_data = container_data[i];
				int slot = slot_data.get("slot", -1);
				if (slot >= 0 && slot < container_capacity && slot_data.has("item")) {
					Ref<Item> item;
					item.instantiate();
					item->deserialize(slot_data["item"]);
					container.write[slot] = item;
				}
			}
		}
	}
}
