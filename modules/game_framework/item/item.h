/**************************************************************************/
/*  item.h                                                                */
/**************************************************************************/

#ifndef ITEM_H
#define ITEM_H

#include "core/string/ustring.h"
#include "core/templates/vector.h"

// 纯数据类，不暴露给 GDScript
class Item {
private:
	uint64_t id = 0;
	int type_id = 0;
	String name;
	int stack_count = 1;
	int max_stack = 1;
	uint64_t container_id = 0; // 所在容器的 ID，0 表示不在任何容器中

	// 如果自己是容器，存储内部物品的指针
	Vector<Item *> contained_items;
	int max_slots = 0; // 0 表示不是容器，>0 表示容器的槽位数

public:
	Item();
	Item(uint64_t p_id, int p_type_id);
	~Item();

	// === ID 和类型 ===
	uint64_t get_id() const { return id; }
	void set_id(uint64_t p_id) { id = p_id; }

	int get_type_id() const { return type_id; }
	void set_type_id(int p_type_id) { type_id = p_type_id; }

	// === 基础属性 ===
	String get_name() const { return name; }
	void set_name(const String &p_name) { name = p_name; }

	int get_stack_count() const { return stack_count; }
	void set_stack_count(int p_count) { stack_count = p_count; }

	int get_max_stack() const { return max_stack; }
	void set_max_stack(int p_max) { max_stack = p_max; }

	// === 容器关系 ===
	uint64_t get_container_id() const { return container_id; }
	void set_container_id(uint64_t p_container_id) { container_id = p_container_id; }

	// === 容器功能 ===
	bool is_container() const { return max_slots > 0; }
	int get_max_slots() const { return max_slots; }
	void set_max_slots(int p_slots) { max_slots = p_slots; }

	int get_item_count() const { return contained_items.size(); }
	const Vector<Item *> &get_contained_items() const { return contained_items; }

	// 添加物品到容器（指定槽位，-1 表示自动寻找空位）
	bool add_item(Item *p_item, int slot = -1);
	// 移除物品
	bool remove_item(Item *p_item);
	// 获取指定槽位的物品
	Item *get_item_at_slot(int slot) const;
	// 查找物品所在槽位，返回 -1 表示未找到
	int find_item_slot(Item *p_item) const;
	// 容器内移动物品到新位置（插入语义，自动压缩空槽）
	bool reorder_item(Item *p_item, int new_index);
	// 清空容器
	void clear_container();

	// === 调试 ===
	String to_string() const;
};

#endif // ITEM_H
