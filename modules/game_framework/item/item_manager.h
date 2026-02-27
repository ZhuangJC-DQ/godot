/**************************************************************************/
/*  item_manager.h                                                        */
/**************************************************************************/

#ifndef ITEM_MANAGER_H
#define ITEM_MANAGER_H

#include "item.h"
#include "item_template_manager.h"

#include "core/object/object.h"
#include "core/templates/hash_map.h"
#include "core/variant/typed_array.h"

class ItemManager : public Object {
	GDCLASS(ItemManager, Object);

private:
	static ItemManager *singleton;

	HashMap<uint64_t, Item *> items;
	uint64_t next_id = 1; // 单机模式下自动分配 ID

protected:
	static void _bind_methods();

public:
	static ItemManager *get_singleton();

	ItemManager();
	~ItemManager();

	// ============================================
	// === 物品生命周期（核心接口）===
	// ============================================

	// 本地生成物品（单机模式）
	uint64_t create_item(int type_id);

	// 服务器指定 ID 生成物品（联机模式预留接口）
	uint64_t create_item_with_id(uint64_t id, int type_id);

	// 删除物品（会自动从容器中移除）
	void destroy_item(uint64_t item_id);

	// 检查物品是否有效
	bool is_valid_item(uint64_t item_id) const;

	// 获取所有物品数量（调试用）
	int get_item_count() const;

	// ============================================
	// === 基础属性读写（暴露给 GDScript）===
	// ============================================

	String get_item_name(uint64_t item_id) const;
	void set_item_name(uint64_t item_id, const String &name);

	int get_item_type(uint64_t item_id) const;

	int get_stack_count(uint64_t item_id) const;
	void set_stack_count(uint64_t item_id, int count);

	int get_max_stack(uint64_t item_id) const;
	void set_max_stack(uint64_t item_id, int max);

	// ============================================
	// === 容器操作（暴露给 GDScript）===
	// ============================================

	// 设置物品为容器
	void set_as_container(uint64_t item_id, int max_slots);

	// 检查是否是容器
	bool is_container(uint64_t item_id) const;

	// 获取容器最大槽位数
	int get_max_slots(uint64_t item_id) const;

	// 添加物品到容器（slot = -1 表示自动寻找空位）
	bool add_to_container(uint64_t item_id, uint64_t container_id, int slot = -1);

	// 从容器中移除物品
	bool remove_from_container(uint64_t item_id);

	// 获取物品所在的容器 ID（0 表示不在任何容器中）
	uint64_t get_container_id(uint64_t item_id) const;

	// 获取物品在容器中的槽位（-1 表示未找到）
	int get_slot_in_container(uint64_t item_id) const;

	// 获取容器中指定槽位的物品 ID（0 表示空槽位）
	uint64_t get_item_at_slot(uint64_t container_id, int slot) const;

	// 获取容器中所有物品的 ID 列表（GDScript Array）
	TypedArray<int> get_container_items(uint64_t container_id) const;

	// 清空容器
	void clear_container(uint64_t container_id);

	// ============================================
	// === 批量操作（性能优化）===
	// ============================================

	// 一次性获取物品的所有数据（Dictionary）
	Dictionary get_item_data(uint64_t item_id) const;

	// 一次性设置物品数据
	void set_item_data(uint64_t item_id, const Dictionary &data);

	// ============================================
	// === 序列化（存档/联机同步）===
	// ============================================

	// 保存所有物品数据到 Dictionary
	Dictionary save_to_dict() const;

	// 从 Dictionary 加载物品数据
	void load_from_dict(const Dictionary &data);

	// ============================================
	// === 调试接口 ===
	// ============================================

	void print_item(uint64_t item_id) const;
	void print_all_items() const;
	void clear_all_items();

private:
	// 内部辅助方法
	Item *get_item(uint64_t item_id) const;
};

#endif // ITEM_MANAGER_H
