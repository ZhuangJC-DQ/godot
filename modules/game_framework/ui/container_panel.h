/**************************************************************************/
/*  container_panel.h                                                     */
/**************************************************************************/

#pragma once

#include "scene/gui/panel_container.h"
#include "scene/gui/grid_container.h"
#include "scene/gui/label.h"

class WorldObject;
class Item;
class ItemSlot;

// ContainerPanel - 容器面板
// 显示WorldObject容器内的物品，支持物品交互
class ContainerPanel : public PanelContainer {
	GDCLASS(ContainerPanel, PanelContainer);

private:
	// 绑定的容器对象
	WorldObject *bound_object = nullptr;

	// UI组件
	Label *title_label = nullptr;
	GridContainer *slot_grid = nullptr;
	Vector<ItemSlot *> slots;

	// 配置
	int32_t columns = 5;
	Size2 slot_size = Size2(64, 64);
	int32_t slot_separation = 4;

	// 内部方法
	void _rebuild_slots();
	void _sync_from_container();
	void _on_slot_clicked(int button_index, int slot_index);
	void _on_slot_item_dropped(ItemSlot *from_slot, Ref<Item> item, int to_slot_index);

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	ContainerPanel();
	virtual ~ContainerPanel();

	// === 绑定容器 ===
	void bind_container(WorldObject *p_object);
	void unbind_container();
	WorldObject *get_bound_object() const { return bound_object; }
	bool has_bound_object() const { return bound_object != nullptr; }

	// === 刷新显示 ===
	void refresh();

	// === 配置 ===
	void set_columns(int32_t p_columns);
	int32_t get_columns() const { return columns; }

	void set_slot_size(const Size2 &p_size);
	Size2 get_slot_size() const { return slot_size; }

	void set_slot_separation(int32_t p_sep);
	int32_t get_slot_separation() const { return slot_separation; }

	void set_title(const String &p_title);
	String get_title() const;

	// === 槽位访问 ===
	ItemSlot *get_slot(int32_t p_index) const;
	int32_t get_slot_count() const { return slots.size(); }
};
