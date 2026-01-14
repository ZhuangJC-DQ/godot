/**************************************************************************/
/*  floating_container_window.h                                           */
/**************************************************************************/

#pragma once

#include "scene/main/window.h"
#include "scene/gui/panel_container.h"
#include "scene/gui/grid_container.h"
#include "scene/gui/label.h"
#include "scene/gui/box_container.h"

class WorldObject;
class Item;
class ItemSlot;

// FloatingContainerWindow - 可拖拽、可调整大小的浮动容器窗口
// 用于显示WorldObject或Item的容器内容
class FloatingContainerWindow : public Window {
	GDCLASS(FloatingContainerWindow, Window);

private:
	// 绑定的容器对象
	WorldObject *bound_object = nullptr;

	// UI组件
	VBoxContainer *main_vbox = nullptr;
	Label *title_label = nullptr;
	GridContainer *slot_grid = nullptr;
	Vector<ItemSlot *> slots;

	// 配置
	int32_t columns = 5;
	Size2 slot_size = Size2(64, 64);
	int32_t slot_separation = 4;

	// 内部方法
	void _setup_ui();
	void _rebuild_slots();
	void _sync_from_container();
	void _on_slot_clicked(int button_index, int slot_index);
	void _on_slot_double_clicked(int slot_index);
	void _on_slot_item_dropped(ItemSlot *from_slot, Ref<Item> item, int to_slot_index);
	void _on_close_requested();

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	FloatingContainerWindow();
	virtual ~FloatingContainerWindow();

	// === 绑定容器 ===
	void bind_container(WorldObject *p_object, const String &p_title = "");
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

	// === 静态工具方法 ===
	// 创建并显示一个浮动容器窗口
	static FloatingContainerWindow *create_and_show(WorldObject *p_object, const String &p_title, Node *p_parent);
};
