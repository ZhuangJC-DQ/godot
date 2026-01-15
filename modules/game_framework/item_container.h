/**************************************************************************/
/*  item_container.h                                                      */
/**************************************************************************/

#pragma once

#include "core/object/ref_counted.h"
#include "core/string/string_name.h"
#include "core/variant/dictionary.h"
#include "core/variant/typed_array.h"
#include "core/variant/variant.h"

class WorldObject;

// 容器类 - 管理 WorldObject 的存储槽位
class ItemContainer : public RefCounted {
	GDCLASS(ItemContainer, RefCounted);

public:
	// 最大嵌套深度（防止无限递归）
	static constexpr int32_t MAX_NESTING_DEPTH = 3;

private:
	// === 核心数据 ===
	Vector<Ref<WorldObject>> slots;  // 存储槽位
	int32_t capacity = 0;            // 容器容量
	int32_t nesting_depth = 0;       // 当前嵌套深度

	// 容器所有者（用于循环检测）
	WorldObject *owner = nullptr;

protected:
	static void _bind_methods();

public:
	ItemContainer();
	virtual ~ItemContainer();

	// === 初始化 ===
	void initialize(int32_t p_capacity, int32_t p_depth = 0);
	void set_owner(WorldObject *p_owner) { owner = p_owner; }
	WorldObject *get_owner() const { return owner; }

	// === 容量管理 ===
	int32_t get_capacity() const { return capacity; }
	int32_t get_used_slots() const;
	int32_t get_empty_slots() const;
	bool is_full() const;
	bool is_empty() const;

	// === 嵌套深度 ===
	int32_t get_nesting_depth() const { return nesting_depth; }
	void set_nesting_depth(int32_t p_depth) { nesting_depth = p_depth; }

	// === 基础槽位操作 ===
	bool add_object(const Ref<WorldObject> &p_object);                    // 添加到第一个可用槽位
	bool add_object_at(int32_t p_slot, const Ref<WorldObject> &p_object); // 添加到指定槽位
	Ref<WorldObject> remove_object(int32_t p_slot);                       // 移除并返回对象
	Ref<WorldObject> get_object(int32_t p_slot) const;                    // 获取槽位对象
	bool set_object(int32_t p_slot, const Ref<WorldObject> &p_object);    // 设置槽位对象（仅设置空槽位）
	void clear();                                                          // 清空容器

	// === 拖拽操作（原子性，防止复制BUG）===
	bool swap_objects(int32_t p_slot_a, int32_t p_slot_b);                // 交换两个槽位的物品
	bool move_object(int32_t p_from_slot, int32_t p_to_slot);             // 移动物品（原子性）
	bool replace_object(int32_t p_slot, const Ref<WorldObject> &p_object); // 强制替换槽位（返回旧对象）

	// === 查找操作 ===
	int32_t find_object(const StringName &p_object_id) const;             // 查找对象槽位
	TypedArray<WorldObject> get_all_objects() const;                      // 获取所有非空对象

	// === 安全检查 ===
	bool can_add_object(const Ref<WorldObject> &p_object) const;          // 检查是否可以添加
	bool would_create_cycle(const Ref<WorldObject> &p_object) const;      // 检查是否会形成循环
	int32_t calculate_max_child_depth(const Ref<WorldObject> &p_object) const; // 计算对象最大深度

	// === 序列化 ===
	Dictionary serialize() const;
	void deserialize(const Dictionary &p_data);
};
