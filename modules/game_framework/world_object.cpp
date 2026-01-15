/**************************************************************************/
/*  world_object.cpp                                                      */
/**************************************************************************/

#include "world_object.h"
#include "item_container.h"
#include "item.h"

#include "core/object/class_db.h"

void WorldObject::_bind_methods() {
	ClassDB::bind_method(D_METHOD("setup", "object_id", "type", "position"), &WorldObject::setup);

	ClassDB::bind_method(D_METHOD("set_object_id", "id"), &WorldObject::set_object_id);
	ClassDB::bind_method(D_METHOD("get_object_id"), &WorldObject::get_object_id);

	ClassDB::bind_method(D_METHOD("set_object_type", "type"), &WorldObject::set_object_type);
	ClassDB::bind_method(D_METHOD("get_object_type"), &WorldObject::get_object_type);

	ClassDB::bind_method(D_METHOD("set_local_position", "position"), &WorldObject::set_local_position);
	ClassDB::bind_method(D_METHOD("get_local_position"), &WorldObject::get_local_position);

	ClassDB::bind_method(D_METHOD("init_container", "capacity", "nesting_depth"), &WorldObject::init_container, DEFVAL(0));
	ClassDB::bind_method(D_METHOD("get_container"), &WorldObject::get_container);
	ClassDB::bind_method(D_METHOD("get_container_capacity"), &WorldObject::get_container_capacity);
	ClassDB::bind_method(D_METHOD("has_container"), &WorldObject::has_container);

	ClassDB::bind_method(D_METHOD("container_add_object", "object"), &WorldObject::container_add_object);
	ClassDB::bind_method(D_METHOD("container_get_object", "slot"), &WorldObject::container_get_object);
	ClassDB::bind_method(D_METHOD("container_remove_object", "slot"), &WorldObject::container_remove_object);
	ClassDB::bind_method(D_METHOD("container_set_object", "slot", "object"), &WorldObject::container_set_object);
	ClassDB::bind_method(D_METHOD("container_swap_objects", "slot_a", "slot_b"), &WorldObject::container_swap_objects);
	ClassDB::bind_method(D_METHOD("container_move_object", "from_slot", "to_slot"), &WorldObject::container_move_object);
	ClassDB::bind_method(D_METHOD("container_replace_object", "slot", "object"), &WorldObject::container_replace_object);

	ClassDB::bind_method(D_METHOD("serialize"), &WorldObject::serialize);
	ClassDB::bind_method(D_METHOD("deserialize", "data"), &WorldObject::deserialize);

	BIND_ENUM_CONSTANT(TYPE_GENERIC);
	BIND_ENUM_CONSTANT(TYPE_CONTAINER);
	BIND_ENUM_CONSTANT(TYPE_RESOURCE);
	BIND_ENUM_CONSTANT(TYPE_FURNITURE);
	BIND_ENUM_CONSTANT(TYPE_CRAFTING);
	BIND_ENUM_CONSTANT(TYPE_INTERACTABLE);
}

WorldObject::WorldObject() = default;

WorldObject::~WorldObject() = default;

void WorldObject::setup(const StringName &p_object_id, ObjectType p_type, const Vector2i &p_position) {
	object_id = p_object_id;
	object_type = p_type;
	local_position = p_position;
}

// ============ 容器系统 ============

void WorldObject::init_container(int32_t p_capacity, int32_t p_nesting_depth) {
	ERR_FAIL_COND_MSG(p_capacity < 0, "Container capacity cannot be negative.");

	container.instantiate();
	container->initialize(p_capacity, p_nesting_depth);
	container->set_owner(this);
}

int32_t WorldObject::get_container_capacity() const {
	if (!has_container()) {
		return 0;
	}
	return container->get_capacity();
}

int32_t WorldObject::get_container_used_slots() const {
	if (!has_container()) {
		return 0;
	}
	return container->get_used_slots();
}

int32_t WorldObject::get_container_empty_slots() const {
	if (!has_container()) {
		return 0;
	}
	return container->get_empty_slots();
}

bool WorldObject::is_container_full() const {
	if (!has_container()) {
		return true;
	}
	return container->is_full();
}

bool WorldObject::is_container_empty() const {
	if (!has_container()) {
		return true;
	}
	return container->is_empty();
}

int32_t WorldObject::get_container_depth() const {
	if (!has_container()) {
		return 0;
	}
	return container->get_nesting_depth();
}

// ============ 容器操作 ============

bool WorldObject::container_add_object(const Ref<WorldObject> &p_object) {
	ERR_FAIL_COND_V_MSG(!has_container(), false, "Object has no container.");
	return container->add_object(p_object);
}

bool WorldObject::container_add_object_at(int32_t p_slot, const Ref<WorldObject> &p_object) {
	ERR_FAIL_COND_V_MSG(!has_container(), false, "Object has no container.");
	return container->add_object_at(p_slot, p_object);
}

Ref<WorldObject> WorldObject::container_remove_object(int32_t p_slot) {
	ERR_FAIL_COND_V_MSG(!has_container(), Ref<WorldObject>(), "Object has no container.");
	return container->remove_object(p_slot);
}

Ref<WorldObject> WorldObject::container_get_object(int32_t p_slot) const {
	ERR_FAIL_COND_V_MSG(!has_container(), Ref<WorldObject>(), "Object has no container.");
	return container->get_object(p_slot);
}

bool WorldObject::container_set_object(int32_t p_slot, const Ref<WorldObject> &p_object) {
	ERR_FAIL_COND_V_MSG(!has_container(), false, "Object has no container.");
	return container->set_object(p_slot, p_object);
}

void WorldObject::container_clear() {
	if (has_container()) {
		container->clear();
	}
}

// ============ 高级容器操作 ============

int32_t WorldObject::container_find_object(const StringName &p_object_id) const {
	ERR_FAIL_COND_V_MSG(!has_container(), -1, "Object has no container.");
	return container->find_object(p_object_id);
}

TypedArray<WorldObject> WorldObject::container_get_all_objects() const {
	if (!has_container()) {
		return TypedArray<WorldObject>();
	}
	return container->get_all_objects();
}

// ============ 容器拖拽操作（原子性）============

bool WorldObject::container_swap_objects(int32_t p_slot_a, int32_t p_slot_b) {
	ERR_FAIL_COND_V_MSG(!has_container(), false, "Object has no container.");
	return container->swap_objects(p_slot_a, p_slot_b);
}

bool WorldObject::container_move_object(int32_t p_from_slot, int32_t p_to_slot) {
	ERR_FAIL_COND_V_MSG(!has_container(), false, "Object has no container.");
	return container->move_object(p_from_slot, p_to_slot);
}

bool WorldObject::container_replace_object(int32_t p_slot, const Ref<WorldObject> &p_object) {
	ERR_FAIL_COND_V_MSG(!has_container(), false, "Object has no container.");
	return container->replace_object(p_slot, p_object);
}

// ============ Item 兼容方法 ============

bool WorldObject::container_add_item(const Ref<Item> &p_item) {
	ERR_FAIL_COND_V_MSG(!has_container(), false, "Object has no container.");
	ERR_FAIL_COND_V_MSG(p_item.is_null(), false, "Cannot add null item.");

	// Item 现在继承 WorldObject，可以直接添加
	Ref<WorldObject> obj = p_item;
	return container->add_object(obj);
}

Ref<Item> WorldObject::container_get_item(int32_t p_slot) const {
	ERR_FAIL_COND_V_MSG(!has_container(), Ref<Item>(), "Object has no container.");

	Ref<WorldObject> obj = container->get_object(p_slot);
	if (obj.is_null()) {
		return Ref<Item>();
	}

	// 尝试转换为 Item
	return Object::cast_to<Item>(obj.ptr());
}

bool WorldObject::container_set_item(int32_t p_slot, const Ref<Item> &p_item) {
	ERR_FAIL_COND_V_MSG(!has_container(), false, "Object has no container.");

	// Item 现在继承 WorldObject，可以直接设置
	Ref<WorldObject> obj = p_item;
	return container->set_object(p_slot, obj);
}

// ============ 交互接口 ============

void WorldObject::interact(Object *p_actor) {
	// 默认交互行为：如果有容器，则在后续由控制器显示容器UI
	// 这里可以添加其他通用的交互逻辑
}

TypedArray<Dictionary> WorldObject::harvest(Object *p_actor) {
	TypedArray<Dictionary> loot;

	// 默认行为：将容器内容作为掉落物
	if (has_container()) {
		TypedArray<WorldObject> objects = container_get_all_objects();
		for (int i = 0; i < objects.size(); i++) {
			Ref<WorldObject> obj = objects[i];
			if (obj.is_valid()) {
				loot.push_back(obj->serialize());
			}
		}
	}

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
		data["container"] = container->serialize();
	}

	return data;
}

void WorldObject::deserialize(const Dictionary &p_data) {
	object_id = p_data.get("object_id", StringName());
	object_type = static_cast<ObjectType>((int)p_data.get("object_type", TYPE_GENERIC));
	local_position.x = p_data.get("position_x", 0);
	local_position.y = p_data.get("position_y", 0);

	if (p_data.has("container")) {
		Dictionary container_data = p_data["container"];
		int32_t capacity = container_data.get("capacity", 0);
		int32_t depth = container_data.get("nesting_depth", 0);

		init_container(capacity, depth);
		container->deserialize(container_data);
	}
}
