/**************************************************************************/
/*  item_container.cpp                                                    */
/**************************************************************************/

#include "item_container.h"
#include "world_object.h"

#include "core/string/print_string.h"

void ItemContainer::_bind_methods() {
	ClassDB::bind_method(D_METHOD("initialize", "capacity", "depth"), &ItemContainer::initialize, DEFVAL(0));
	ClassDB::bind_method(D_METHOD("get_capacity"), &ItemContainer::get_capacity);
	ClassDB::bind_method(D_METHOD("get_used_slots"), &ItemContainer::get_used_slots);
	ClassDB::bind_method(D_METHOD("get_empty_slots"), &ItemContainer::get_empty_slots);
	ClassDB::bind_method(D_METHOD("is_full"), &ItemContainer::is_full);
	ClassDB::bind_method(D_METHOD("is_empty"), &ItemContainer::is_empty);

	ClassDB::bind_method(D_METHOD("add_object", "object"), &ItemContainer::add_object);
	ClassDB::bind_method(D_METHOD("add_object_at", "slot", "object"), &ItemContainer::add_object_at);
	ClassDB::bind_method(D_METHOD("remove_object", "slot"), &ItemContainer::remove_object);
	ClassDB::bind_method(D_METHOD("get_object", "slot"), &ItemContainer::get_object);
	ClassDB::bind_method(D_METHOD("set_object", "slot", "object"), &ItemContainer::set_object);
	ClassDB::bind_method(D_METHOD("clear"), &ItemContainer::clear);

	ClassDB::bind_method(D_METHOD("swap_objects", "slot_a", "slot_b"), &ItemContainer::swap_objects);
	ClassDB::bind_method(D_METHOD("move_object", "from_slot", "to_slot"), &ItemContainer::move_object);
	ClassDB::bind_method(D_METHOD("replace_object", "slot", "object"), &ItemContainer::replace_object);

	ClassDB::bind_method(D_METHOD("find_object", "object_id"), &ItemContainer::find_object);
	ClassDB::bind_method(D_METHOD("get_all_objects"), &ItemContainer::get_all_objects);

	ClassDB::bind_method(D_METHOD("can_add_object", "object"), &ItemContainer::can_add_object);

	BIND_CONSTANT(MAX_NESTING_DEPTH);
}

ItemContainer::ItemContainer() = default;

ItemContainer::~ItemContainer() = default;

// ============ 初始化 ============

void ItemContainer::initialize(int32_t p_capacity, int32_t p_depth) {
	ERR_FAIL_COND_MSG(p_capacity < 0, "Container capacity cannot be negative.");
	ERR_FAIL_COND_MSG(p_depth < 0, "Container depth cannot be negative.");

	capacity = p_capacity;
	nesting_depth = p_depth;
	slots.resize(p_capacity);

	for (int32_t i = 0; i < p_capacity; i++) {
		slots.write[i] = Ref<WorldObject>();
	}
}

// ============ 容量管理 ============

int32_t ItemContainer::get_used_slots() const {
	int32_t count = 0;
	for (const Ref<WorldObject> &obj : slots) {
		if (obj.is_valid()) {
			count++;
		}
	}
	return count;
}

int32_t ItemContainer::get_empty_slots() const {
	return capacity - get_used_slots();
}

bool ItemContainer::is_full() const {
	return get_used_slots() >= capacity;
}

bool ItemContainer::is_empty() const {
	return get_used_slots() == 0;
}

// ============ 基础槽位操作 ============

bool ItemContainer::add_object(const Ref<WorldObject> &p_object) {
	ERR_FAIL_COND_V_MSG(p_object.is_null(), false, "Cannot add null object.");

	// 安全检查
	if (!can_add_object(p_object)) {
		return false;
	}

	// 查找空槽位
	for (int32_t i = 0; i < capacity; i++) {
		if (slots[i].is_null()) {
			slots.write[i] = p_object;
			return true;
		}
	}

	return false; // 容器已满
}

bool ItemContainer::add_object_at(int32_t p_slot, const Ref<WorldObject> &p_object) {
	ERR_FAIL_INDEX_V(p_slot, capacity, false);
	ERR_FAIL_COND_V_MSG(p_object.is_null(), false, "Cannot add null object.");

	// 安全检查
	if (!can_add_object(p_object)) {
		return false;
	}

	// 检查槽位是否为空
	if (slots[p_slot].is_valid()) {
		return false; // 槽位已被占用
	}

	slots.write[p_slot] = p_object;
	return true;
}

Ref<WorldObject> ItemContainer::remove_object(int32_t p_slot) {
	ERR_FAIL_INDEX_V(p_slot, capacity, Ref<WorldObject>());

	Ref<WorldObject> obj = slots[p_slot];
	if (obj.is_valid()) {
		slots.write[p_slot] = Ref<WorldObject>();
	}
	return obj;
}

Ref<WorldObject> ItemContainer::get_object(int32_t p_slot) const {
	ERR_FAIL_INDEX_V(p_slot, capacity, Ref<WorldObject>());
	return slots[p_slot];
}

bool ItemContainer::set_object(int32_t p_slot, const Ref<WorldObject> &p_object) {
	ERR_FAIL_INDEX_V(p_slot, capacity, false);

	// 【修复】仅允许设置空槽位，防止意外覆盖
	if (slots[p_slot].is_valid()) {
		ERR_PRINT(vformat("Cannot set_object: slot %d is already occupied. Use replace_object or swap_objects instead.", p_slot));
		return false;
	}

	// 如果设置非空对象，需要安全检查
	if (p_object.is_valid() && !can_add_object(p_object)) {
		return false;
	}

	slots.write[p_slot] = p_object;
	return true;
}

void ItemContainer::clear() {
	for (int32_t i = 0; i < slots.size(); i++) {
		if (slots[i].is_valid()) {
			slots.write[i] = Ref<WorldObject>();
		}
	}
}

// ============ 拖拽操作（原子性）============

bool ItemContainer::swap_objects(int32_t p_slot_a, int32_t p_slot_b) {
	ERR_FAIL_INDEX_V(p_slot_a, capacity, false);
	ERR_FAIL_INDEX_V(p_slot_b, capacity, false);

	// 允许相同槽位（无操作）
	if (p_slot_a == p_slot_b) {
		return true;
	}

	Ref<WorldObject> obj_a = slots[p_slot_a];
	Ref<WorldObject> obj_b = slots[p_slot_b];

	// 如果都是空槽位，无需交换
	if (obj_a.is_null() && obj_b.is_null()) {
		return true;
	}

	// 【关键】原子性交换，防止复制BUG
	slots.write[p_slot_a] = obj_b;
	slots.write[p_slot_b] = obj_a;

	return true;
}

bool ItemContainer::move_object(int32_t p_from_slot, int32_t p_to_slot) {
	ERR_FAIL_INDEX_V(p_from_slot, capacity, false);
	ERR_FAIL_INDEX_V(p_to_slot, capacity, false);

	// 源槽位必须有物品
	if (slots[p_from_slot].is_null()) {
		ERR_PRINT(vformat("Cannot move: source slot %d is empty.", p_from_slot));
		return false;
	}

	// 目标槽位必须为空
	if (slots[p_to_slot].is_valid()) {
		ERR_PRINT(vformat("Cannot move: target slot %d is occupied. Use swap_objects instead.", p_to_slot));
		return false;
	}

	// 【关键】原子性移动：先获取对象，再清空源槽位，最后设置目标槽位
	Ref<WorldObject> obj = slots[p_from_slot];
	slots.write[p_from_slot] = Ref<WorldObject>();
	slots.write[p_to_slot] = obj;

	return true;
}

bool ItemContainer::replace_object(int32_t p_slot, const Ref<WorldObject> &p_object) {
	ERR_FAIL_INDEX_V(p_slot, capacity, false);

	// 如果设置非空对象，需要安全检查
	if (p_object.is_valid() && !can_add_object(p_object)) {
		return false;
	}

	// 【关键】强制替换，旧对象会被丢弃（Ref自动管理内存）
	slots.write[p_slot] = p_object;
	return true;
}

// ============ 查找操作 ============

int32_t ItemContainer::find_object(const StringName &p_object_id) const {
	for (int32_t i = 0; i < slots.size(); i++) {
		if (slots[i].is_valid() && slots[i]->get_object_id() == p_object_id) {
			return i;
		}
	}
	return -1;
}

TypedArray<WorldObject> ItemContainer::get_all_objects() const {
	TypedArray<WorldObject> objects;
	for (const Ref<WorldObject> &obj : slots) {
		if (obj.is_valid()) {
			objects.push_back(obj);
		}
	}
	return objects;
}

// ============ 安全检查 ============

bool ItemContainer::can_add_object(const Ref<WorldObject> &p_object) const {
	ERR_FAIL_COND_V_MSG(p_object.is_null(), false, "Object is null.");

	// 检查是否会形成循环引用
	if (would_create_cycle(p_object)) {
		ERR_PRINT("Cannot add object: would create circular reference.");
		return false;
	}

	// 检查嵌套深度
	int32_t child_depth = calculate_max_child_depth(p_object);
	if (nesting_depth + child_depth + 1 > MAX_NESTING_DEPTH) {
		ERR_PRINT(vformat("Cannot add object: would exceed max nesting depth (%d). Current: %d, Child: %d",
			MAX_NESTING_DEPTH, nesting_depth, child_depth));
		return false;
	}

	return true;
}

bool ItemContainer::would_create_cycle(const Ref<WorldObject> &p_object) const {
	if (p_object.is_null() || owner == nullptr) {
		return false;
	}

	// 检查是否试图把容器放入自己
	if (p_object.ptr() == owner) {
		return true;
	}

	// 检查是否试图把祖先容器放入后代容器
	// 向上遍历所有父容器
	WorldObject *current = owner;
	while (current != nullptr) {
		if (current == p_object.ptr()) {
			return true;
		}

		// 获取父容器（通过遍历所有容器查找包含当前对象的容器）
		// 注意：这需要 WorldObject 提供获取父容器的方法
		// 暂时简化处理
		break;
	}

	return false;
}

int32_t ItemContainer::calculate_max_child_depth(const Ref<WorldObject> &p_object) const {
	if (p_object.is_null() || !p_object->has_container()) {
		return 0;
	}

	Ref<ItemContainer> child_container = p_object->get_container();
	if (child_container.is_null()) {
		return 0;
	}

	int32_t max_depth = 0;
	TypedArray<WorldObject> child_objects = child_container->get_all_objects();

	for (int i = 0; i < child_objects.size(); i++) {
		Ref<WorldObject> child = child_objects[i];
		if (child.is_valid()) {
			int32_t child_depth = calculate_max_child_depth(child);
			max_depth = MAX(max_depth, child_depth);
		}
	}

	return max_depth + 1;
}

// ============ 序列化 ============

Dictionary ItemContainer::serialize() const {
	Dictionary data;
	data["capacity"] = capacity;
	data["nesting_depth"] = nesting_depth;

	Array slots_data;
	for (int32_t i = 0; i < slots.size(); i++) {
		const Ref<WorldObject> &obj = slots[i];
		if (obj.is_valid()) {
			Dictionary slot_data;
			slot_data["slot"] = i;
			slot_data["object_type"] = obj->get_class();
			slot_data["object_data"] = obj->serialize();
			slots_data.push_back(slot_data);
		}
	}

	if (!slots_data.is_empty()) {
		data["slots"] = slots_data;
	}

	return data;
}

void ItemContainer::deserialize(const Dictionary &p_data) {
	int32_t cap = p_data.get("capacity", 0);
	int32_t depth = p_data.get("nesting_depth", 0);
	initialize(cap, depth);

	if (p_data.has("slots")) {
		Array slots_data = p_data["slots"];
		for (int i = 0; i < slots_data.size(); i++) {
			Dictionary slot_data = slots_data[i];
			int32_t slot = slot_data.get("slot", -1);

			if (slot >= 0 && slot < capacity && slot_data.has("object_data")) {
				String object_type = slot_data.get("object_type", "WorldObject");
				Dictionary object_data = slot_data["object_data"];

				// 创建对应类型的对象
				Ref<WorldObject> obj;
				obj.instantiate();
				obj->deserialize(object_data);

				slots.write[slot] = obj;
			}
		}
	}
}
