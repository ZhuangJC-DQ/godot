/**************************************************************************/
/*  world_item.h                                                          */
/**************************************************************************/

#pragma once

#include "core/object/gdvirtual.gen.inc"
#include "core/object/ref_counted.h"
#include "core/variant/dictionary.h"
#include "core/variant/typed_array.h"

// 容器槽位数据
struct ItemStack {
	StringName item_id;
	int32_t quantity = 0;
	Dictionary metadata;

	ItemStack() = default;
	ItemStack(const StringName &p_id, int32_t p_qty) :
			item_id(p_id), quantity(p_qty) {}

	bool is_empty() const { return item_id == StringName() || quantity <= 0; }
	void clear() {
		item_id = StringName();
		quantity = 0;
		metadata.clear();
	}
};

// 世界物品基类
class WorldItem : public RefCounted {
	GDCLASS(WorldItem, RefCounted);

public:
	enum ItemType {
		TYPE_GENERIC,    // 通用物品
		TYPE_CONTAINER,  // 容器类（箱子、柜子）
		TYPE_RESOURCE,   // 资源类（树木、矿石）
		TYPE_FURNITURE,  // 家具类
		TYPE_MAX
	};

private:
	StringName item_id;
	ItemType item_type = TYPE_GENERIC;
	Vector2i local_position; // 区块内坐标

	// 容器数据
	Vector<ItemStack> container;
	int32_t container_capacity = 0;

protected:
	static void _bind_methods();

	// 虚函数声明（可被GDScript或子类重写）
	GDVIRTUAL1(_on_interact, Object *)
	GDVIRTUAL2(_on_harvest, Object *, TypedArray<Dictionary>)

public:
	WorldItem();
	virtual ~WorldItem();

	// 初始化
	void setup(const StringName &p_item_id, ItemType p_type, const Vector2i &p_position);

	// 基础属性
	StringName get_item_id() const { return item_id; }
	ItemType get_item_type() const { return item_type; }
	Vector2i get_local_position() const { return local_position; }
	void set_local_position(const Vector2i &p_pos) { local_position = p_pos; }

	// 容器系统
	void init_container(int32_t p_capacity);
	int32_t get_container_capacity() const { return container_capacity; }
	int32_t get_container_used_slots() const;
	bool has_container() const { return container_capacity > 0; }

	bool container_add(const StringName &p_item_id, int32_t p_quantity, const Dictionary &p_metadata = Dictionary());
	ItemStack container_remove(int32_t p_slot, int32_t p_quantity = -1);
	ItemStack container_get(int32_t p_slot) const;
	void container_clear();

	// 交互接口
	void interact(Object *p_actor);
	TypedArray<Dictionary> harvest(Object *p_actor);

	// 序列化
	Dictionary serialize() const;
	void deserialize(const Dictionary &p_data);
};

VARIANT_ENUM_CAST(WorldItem::ItemType);
