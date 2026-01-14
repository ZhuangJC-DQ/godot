/**************************************************************************/
/*  floating_container_window.cpp                                         */
/**************************************************************************/

#include "floating_container_window.h"

#include "item_slot.h"
#include "../world_object.h"
#include "../item.h"
#include "scene/gui/margin_container.h"
#include "scene/gui/panel_container.h"

void FloatingContainerWindow::_bind_methods() {
	ClassDB::bind_method(D_METHOD("bind_container", "object", "title"), &FloatingContainerWindow::bind_container, DEFVAL(""));
	ClassDB::bind_method(D_METHOD("unbind_container"), &FloatingContainerWindow::unbind_container);
	ClassDB::bind_method(D_METHOD("get_bound_object"), &FloatingContainerWindow::get_bound_object);
	ClassDB::bind_method(D_METHOD("refresh"), &FloatingContainerWindow::refresh);

	ClassDB::bind_method(D_METHOD("set_columns", "columns"), &FloatingContainerWindow::set_columns);
	ClassDB::bind_method(D_METHOD("get_columns"), &FloatingContainerWindow::get_columns);

	ClassDB::bind_method(D_METHOD("set_slot_size", "size"), &FloatingContainerWindow::set_slot_size);
	ClassDB::bind_method(D_METHOD("get_slot_size"), &FloatingContainerWindow::get_slot_size);

	// 信号
	ADD_SIGNAL(MethodInfo("item_double_clicked", PropertyInfo(Variant::OBJECT, "item"), PropertyInfo(Variant::INT, "slot_index")));
}

void FloatingContainerWindow::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
			_setup_ui();
			if (bound_object) {
				_rebuild_slots();
				_sync_from_container();
			}
		} break;

		case NOTIFICATION_PREDELETE: {
			unbind_container();
		} break;
	}
}

FloatingContainerWindow::FloatingContainerWindow() {
	// 窗口配置
	set_title("Container");
	set_min_size(Size2(300, 200));
	set_size(Size2(400, 300));
	// Window 默认就可以调整大小

	// 关闭按钮
	connect("close_requested", callable_mp(this, &FloatingContainerWindow::_on_close_requested));
}

FloatingContainerWindow::~FloatingContainerWindow() {
	unbind_container();
}

void FloatingContainerWindow::_setup_ui() {
	// 创建主容器
	MarginContainer *margin = memnew(MarginContainer);
	margin->add_theme_constant_override("margin_left", 8);
	margin->add_theme_constant_override("margin_right", 8);
	margin->add_theme_constant_override("margin_top", 8);
	margin->add_theme_constant_override("margin_bottom", 8);
	add_child(margin);

	// 创建垂直布局
	main_vbox = memnew(VBoxContainer);
	main_vbox->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	main_vbox->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	margin->add_child(main_vbox);

	// 标题标签
	title_label = memnew(Label);
	title_label->set_text("Container");
	title_label->set_horizontal_alignment(HORIZONTAL_ALIGNMENT_CENTER);
	main_vbox->add_child(title_label);

	// 分隔符
	Control *spacer = memnew(Control);
	spacer->set_custom_minimum_size(Size2(0, 4));
	main_vbox->add_child(spacer);

	// 槽位网格容器
	PanelContainer *panel = memnew(PanelContainer);
	panel->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	panel->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	main_vbox->add_child(panel);

	MarginContainer *grid_margin = memnew(MarginContainer);
	grid_margin->add_theme_constant_override("margin_left", 4);
	grid_margin->add_theme_constant_override("margin_right", 4);
	grid_margin->add_theme_constant_override("margin_top", 4);
	grid_margin->add_theme_constant_override("margin_bottom", 4);
	panel->add_child(grid_margin);

	slot_grid = memnew(GridContainer);
	slot_grid->set_columns(columns);
	slot_grid->add_theme_constant_override("h_separation", slot_separation);
	slot_grid->add_theme_constant_override("v_separation", slot_separation);
	slot_grid->set_h_size_flags(Control::SIZE_EXPAND_FILL);
	slot_grid->set_v_size_flags(Control::SIZE_EXPAND_FILL);
	grid_margin->add_child(slot_grid);
}

void FloatingContainerWindow::bind_container(WorldObject *p_object, const String &p_title) {
	if (bound_object == p_object) {
		return;
	}

	unbind_container();
	bound_object = p_object;

	if (bound_object) {
		// 设置标题
		String title = p_title;
		if (title.is_empty()) {
			title = String(bound_object->get_object_id());
		}
		set_title(title);
		if (title_label) {
			title_label->set_text(title);
		}

		_rebuild_slots();
		_sync_from_container();
	}
}

void FloatingContainerWindow::unbind_container() {
	if (bound_object) {
		bound_object = nullptr;

		// 清空所有槽位
		for (ItemSlot *slot : slots) {
			if (slot) {
				slot->clear_item();
			}
		}
	}
}

void FloatingContainerWindow::refresh() {
	if (bound_object) {
		_sync_from_container();
	}
}

void FloatingContainerWindow::set_columns(int32_t p_columns) {
	columns = MAX(1, p_columns);
	if (slot_grid) {
		slot_grid->set_columns(columns);
	}
}

void FloatingContainerWindow::set_slot_size(const Size2 &p_size) {
	slot_size = p_size;
	for (ItemSlot *slot : slots) {
		if (slot) {
			slot->set_custom_minimum_size(slot_size);
		}
	}
}

void FloatingContainerWindow::_rebuild_slots() {
	if (!slot_grid || !bound_object) {
		return;
	}

	int32_t capacity = bound_object->get_container_capacity();

	// 移除多余的槽位
	while (slots.size() > capacity) {
		ItemSlot *slot = slots[slots.size() - 1];
		slots.remove_at(slots.size() - 1);
		if (slot) {
			slot_grid->remove_child(slot);
			memdelete(slot);
		}
	}

	// 添加不足的槽位
	while (slots.size() < capacity) {
		ItemSlot *slot = memnew(ItemSlot);
		slot->set_custom_minimum_size(slot_size);
		slot->set_slot_index(slots.size());

		// 连接信号
		int slot_index = slots.size();
		slot->connect("clicked", callable_mp(this, &FloatingContainerWindow::_on_slot_clicked).bind(slot_index));
		slot->connect("double_clicked", callable_mp(this, &FloatingContainerWindow::_on_slot_double_clicked).bind(slot_index));
		slot->connect("item_dropped", callable_mp(this, &FloatingContainerWindow::_on_slot_item_dropped).bind(slot_index));

		slot_grid->add_child(slot);
		slots.push_back(slot);
	}
}

void FloatingContainerWindow::_sync_from_container() {
	if (!bound_object) {
		return;
	}

	// 直接从容器同步物品到UI
	for (int i = 0; i < slots.size(); i++) {
		if (slots[i]) {
			Ref<Item> item = bound_object->container_get_item(i);
			slots[i]->set_item(item);
		}
	}
}

void FloatingContainerWindow::_on_slot_clicked(int button_index, int slot_index) {
	// 右键使用物品或其他操作
	if (button_index == 2) {  // MOUSE_BUTTON_RIGHT
		if (slot_index >= 0 && slot_index < slots.size() && slots[slot_index]) {
			ItemSlot *slot = slots[slot_index];
			if (slot->has_item()) {
				// 可以在这里添加右键菜单等
			}
		}
	}
}

void FloatingContainerWindow::_on_slot_double_clicked(int slot_index) {
	if (!bound_object || slot_index < 0 || slot_index >= slots.size()) {
		return;
	}

	ItemSlot *slot = slots[slot_index];
	if (!slot || !slot->has_item()) {
		return;
	}

	Ref<Item> item = slot->get_item();
	if (item.is_null()) {
		return;
	}

	// 检查Item是否有容器
	if (item->has_container() && item->get_container_capacity() > 0) {
		// 打开Item的容器在新窗口中
		String title = String(item->get_display_name());
		if (title.is_empty()) {
			title = String(item->get_item_id());
		}
		title = title + " (Container)";

		// 创建新的浮动窗口
		FloatingContainerWindow *new_window = create_and_show(item.ptr(), title, get_parent());

		// 发出信号
		emit_signal("item_double_clicked", item, slot_index);
	}
}

void FloatingContainerWindow::_on_slot_item_dropped(ItemSlot *from_slot, Ref<Item> item, int to_slot_index) {
	if (!bound_object || !from_slot) {
		return;
	}

	int from_index = from_slot->get_slot_index();

	// 如果是同一个容器内的移动
	// 交换两个槽位的物品
	if (from_index >= 0 && from_index < bound_object->get_container_capacity()) {
		Ref<Item> target_item = bound_object->container_get_item(to_slot_index);

		bound_object->container_set_item(to_slot_index, item);
		bound_object->container_set_item(from_index, target_item);

		_sync_from_container();
	}
}

void FloatingContainerWindow::_on_close_requested() {
	// 窗口关闭时清理
	unbind_container();
	queue_free();
}

// === 静态工具方法 ===

FloatingContainerWindow *FloatingContainerWindow::create_and_show(WorldObject *p_object, const String &p_title, Node *p_parent) {
	ERR_FAIL_NULL_V(p_object, nullptr);
	ERR_FAIL_NULL_V(p_parent, nullptr);

	FloatingContainerWindow *window = memnew(FloatingContainerWindow);
	p_parent->add_child(window);

	window->bind_container(p_object, p_title);
	window->popup_centered();

	return window;
}
