/**************************************************************************/
/*  world_object.h                                                        */
/**************************************************************************/

#pragma once

#include "item.h"

#include "core/object/gdvirtual.gen.inc"
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
	Vector<Ref<Item>> container;     // 使用 Item 类存储物品
	int32_t container_capacity = 0;

protected:
	static void _bind_methods();

	// 虚函数声明（可被GDScript或子类重写）
	GDVIRTUAL1(_on_interact, Object *)
	GDVIRTUAL2(_on_harvest, Object *, TypedArray<Dictionary>)
	GDVIRTUAL2(_on_item_added, int32_t, Ref<Item>)
	GDVIRTUAL2(_on_item_removed, int32_t, Ref<Item>)

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
	void init_container(int32_t p_capacity);
	int32_t get_container_capacity() const { return container_capacity; }
	int32_t get_container_used_slots() const;
	int32_t get_container_empty_slots() const;
	bool has_container() const { return container_capacity > 0; }
	bool is_container_full() const;
	bool is_container_empty() const;

	// 容器操作
	bool container_add_item(const Ref<Item> &p_item);              // 添加物品到第一个可用槽位
	bool container_add_item_at(int32_t p_slot, const Ref<Item> &p_item);  // 添加到指定槽位
	Ref<Item> container_remove_item(int32_t p_slot);               // 移除并返回物品
	Ref<Item> container_get_item(int32_t p_slot) const;            // 获取槽位物品
	bool container_set_item(int32_t p_slot, const Ref<Item> &p_item);     // 设置槽位物品
	void container_clear();                                         // 清空容器

	// 高级容器操作
	int32_t container_find_item(const StringName &p_item_id) const;       // 查找物品槽位
	int32_t container_count_item(const StringName &p_item_id) const;      // 统计物品数量
	bool container_has_item(const StringName &p_item_id, int32_t p_quantity = 1) const;
	int32_t container_add_items(const StringName &p_item_id, int32_t p_quantity);  // 返回剩余数量
	int32_t container_remove_items(const StringName &p_item_id, int32_t p_quantity);  // 返回实际移除数量
	TypedArray<Ref<Item>> container_get_all_items() const;         // 获取所有非空物品

	// 物品堆叠
	bool container_try_stack(const Ref<Item> &p_item);             // 尝试堆叠到现有物品

	// === 交互接口 ===
	void interact(Object *p_actor);
	TypedArray<Dictionary> harvest(Object *p_actor);

	// === 序列化 ===
	Dictionary serialize() const;
	void deserialize(const Dictionary &p_data);
};

VARIANT_ENUM_CAST(WorldObject::ObjectType);
