/**************************************************************************/
/*  world_object.h                                                        */
/**************************************************************************/

#pragma once

#include "item_container.h"

#include "core/object/ref_counted.h"
#include "core/variant/dictionary.h"
#include "core/variant/typed_array.h"

// 世界物体基类 - 表示世界中所有可交互的对象
class WorldObject : public RefCounted {
	GDCLASS(WorldObject, RefCounted);

public:
	enum ObjectType {
		TYPE_GENERIC,    // 通用物体
		TYPE_CONTAINER,  // 容器类（箱子、柜子）
		TYPE_RESOURCE,   // 资源类（树木、矿石）
		TYPE_FURNITURE,  // 家具类
		TYPE_CRAFTING,   // 制作台类
		TYPE_INTERACTABLE, // 可交互类（门、开关）
		TYPE_MAX
	};

private:
	// === 核心标识 ===
	StringName object_id;            // 物体唯一ID（对应数据表）
	ObjectType object_type = TYPE_GENERIC;
	Vector2i local_position;         // 区块内坐标

	// === 容器系统 ===
	Ref<ItemContainer> container;        // 容器（可选，默认为null）

protected:
	static void _bind_methods();

public:
	WorldObject();
	virtual ~WorldObject();

	// === 初始化 ===
	void setup(const StringName &p_object_id, ObjectType p_type, const Vector2i &p_position);

	// === 基础属性 ===
	void set_object_id(const StringName &p_id) { object_id = p_id; }
	StringName get_object_id() const { return object_id; }

	void set_object_type(ObjectType p_type) { object_type = p_type; }
	ObjectType get_object_type() const { return object_type; }

	void set_local_position(const Vector2i &p_pos) { local_position = p_pos; }
	Vector2i get_local_position() const { return local_position; }

	// === 容器系统 ===
	void init_container(int32_t p_capacity, int32_t p_nesting_depth = 0);
	Ref<ItemContainer> get_container() const { return container; }
	int32_t get_container_capacity() const;
	int32_t get_container_used_slots() const;
	int32_t get_container_empty_slots() const;
	bool has_container() const { return container.is_valid(); }
	bool is_container_full() const;
	bool is_container_empty() const;
	int32_t get_container_depth() const;

	// 容器操作 - 通用对象
	bool container_add_object(const Ref<WorldObject> &p_object);              // 添加对象到第一个可用槽位
	bool container_add_object_at(int32_t p_slot, const Ref<WorldObject> &p_object);  // 添加到指定槽位
	Ref<WorldObject> container_remove_object(int32_t p_slot);                 // 移除并返回对象
	Ref<WorldObject> container_get_object(int32_t p_slot) const;              // 获取槽位对象
	bool container_set_object(int32_t p_slot, const Ref<WorldObject> &p_object);     // 设置槽位对象（仅空槽位）
	void container_clear();                                                    // 清空容器

	// 容器拖拽操作（原子性）
	bool container_swap_objects(int32_t p_slot_a, int32_t p_slot_b);          // 交换两个槽位
	bool container_move_object(int32_t p_from_slot, int32_t p_to_slot);       // 移动物品到空槽位
	bool container_replace_object(int32_t p_slot, const Ref<WorldObject> &p_object); // 强制替换

	// 高级容器操作
	int32_t container_find_object(const StringName &p_object_id) const;       // 查找对象槽位
	TypedArray<WorldObject> container_get_all_objects() const;                 // 获取所有非空对象

	// === Item 兼容方法 ===
	// 这些方法提供向后兼容，将 Item 视为 WorldObject 的特化
	bool container_add_item(const Ref<class Item> &p_item);                    // 添加物品（兼容）
	Ref<class Item> container_get_item(int32_t p_slot) const;                  // 获取物品（兼容）
	bool container_set_item(int32_t p_slot, const Ref<class Item> &p_item);    // 设置物品（兼容）

	// === 交互接口 ===
	void interact(Object *p_actor);
	TypedArray<Dictionary> harvest(Object *p_actor);

	// === 序列化 ===
	virtual Dictionary serialize() const;
	virtual void deserialize(const Dictionary &p_data);
};

VARIANT_ENUM_CAST(WorldObject::ObjectType);
