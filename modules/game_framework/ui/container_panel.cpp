/**************************************************************************/
/*  container_panel.cpp                                                   */
/**************************************************************************/

#include "container_panel.h"

#include "item_slot.h"
#include "../world_object.h"
#include "../item.h"
#include "scene/gui/box_container.h"
#include "scene/gui/margin_container.h"

void ContainerPanel::_bind_methods() {
	ClassDB::bind_method(D_METHOD("bind_container", "object"), &ContainerPanel::bind_container);
	ClassDB::bind_method(D_METHOD("unbind_container"), &ContainerPanel::unbind_container);
	ClassDB::bind_method(D_METHOD("get_bound_object"), &ContainerPanel::get_bound_object);
	ClassDB::bind_method(D_METHOD("has_bound_object"), &ContainerPanel::has_bound_object);

	ClassDB::bind_method(D_METHOD("refresh"), &ContainerPanel::refresh);

	ClassDB::bind_method(D_METHOD("set_columns", "columns"), &ContainerPanel::set_columns);
	ClassDB::bind_method(D_METHOD("get_columns"), &ContainerPanel::get_columns);

	ClassDB::bind_method(D_METHOD("set_slot_size", "size"), &ContainerPanel::set_slot_size);
	ClassDB::bind_method(D_METHOD("get_slot_size"), &ContainerPanel::get_slot_size);

	ClassDB::bind_method(D_METHOD("set_slot_separation", "separation"), &ContainerPanel::set_slot_separation);
	ClassDB::bind_method(D_METHOD("get_slot_separation"), &ContainerPanel::get_slot_separation);

	ClassDB::bind_method(D_METHOD("set_title", "title"), &ContainerPanel::set_title);
	ClassDB::bind_method(D_METHOD("get_title"), &ContainerPanel::get_title);

	ClassDB::bind_method(D_METHOD("set_title_label_path", "path"), &ContainerPanel::set_title_label_path);
	ClassDB::bind_method(D_METHOD("get_title_label_path"), &ContainerPanel::get_title_label_path);

	ClassDB::bind_method(D_METHOD("set_slot_grid_path", "path"), &ContainerPanel::set_slot_grid_path);
	ClassDB::bind_method(D_METHOD("get_slot_grid_path"), &ContainerPanel::get_slot_grid_path);

	ClassDB::bind_method(D_METHOD("get_slot", "index"), &ContainerPanel::get_slot);
	ClassDB::bind_method(D_METHOD("get_slot_count"), &ContainerPanel::get_slot_count);

	// 属性
	ADD_PROPERTY(PropertyInfo(Variant::INT, "columns", PROPERTY_HINT_RANGE, "1,20"), "set_columns", "get_columns");
	ADD_PROPERTY(PropertyInfo(Variant::VECTOR2, "slot_size"), "set_slot_size", "get_slot_size");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "slot_separation"), "set_slot_separation", "get_slot_separation");
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "title"), "set_title", "get_title");

	ADD_GROUP("Node Paths", "");
	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "title_label_path"), "set_title_label_path", "get_title_label_path");
	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "slot_grid_path"), "set_slot_grid_path", "get_slot_grid_path");

	// 信号
	ADD_SIGNAL(MethodInfo("container_bound", PropertyInfo(Variant::OBJECT, "object")));
	ADD_SIGNAL(MethodInfo("container_unbound"));
	ADD_SIGNAL(MethodInfo("slot_clicked",
			PropertyInfo(Variant::INT, "slot_index"),
			PropertyInfo(Variant::INT, "button_index")));
	ADD_SIGNAL(MethodInfo("item_moved",
			PropertyInfo(Variant::INT, "from_slot"),
			PropertyInfo(Variant::INT, "to_slot")));
	ADD_SIGNAL(MethodInfo("item_used",
			PropertyInfo(Variant::INT, "slot_index"),
			PropertyInfo(Variant::OBJECT, "item")));
}

void ContainerPanel::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
			_get_ui_nodes();
		} break;
	}
}

ContainerPanel::ContainerPanel() {
}

ContainerPanel::~ContainerPanel() {
	unbind_container();
}

void ContainerPanel::_get_ui_nodes() {
	// 通过NodePath获取场景中的UI节点
	title_label = Object::cast_to<Label>(get_node_or_null(title_label_path));
	slot_grid = Object::cast_to<GridContainer>(get_node_or_null(slot_grid_path));

	// 检查必需节点
	if (!title_label) {
		ERR_PRINT(vformat("ContainerPanel: 无法找到TitleLabel节点，路径: %s。请从场景文件实例化ContainerPanel。", title_label_path));
	}
	if (!slot_grid) {
		ERR_PRINT(vformat("ContainerPanel: 无法找到SlotGrid节点，路径: %s。请从场景文件实例化ContainerPanel。", slot_grid_path));
	} else {
		// 应用配置
		slot_grid->set_columns(columns);
		slot_grid->add_theme_constant_override("h_separation", slot_separation);
		slot_grid->add_theme_constant_override("v_separation", slot_separation);
	}
}

void ContainerPanel::bind_container(WorldObject *p_object) {
	if (bound_object == p_object) {
		return;
	}

	unbind_container();
	bound_object = p_object;

	if (bound_object) {
		_rebuild_slots();
		_sync_from_container();
		emit_signal("container_bound", bound_object);
	}
}

void ContainerPanel::unbind_container() {
	if (bound_object) {
		bound_object = nullptr;

		// 清空所有槽位
		for (ItemSlot *slot : slots) {
			slot->clear_item();
		}

		emit_signal("container_unbound");
	}
}

void ContainerPanel::refresh() {
	if (bound_object) {
		_sync_from_container();
	}
}

void ContainerPanel::set_columns(int32_t p_columns) {
	columns = MAX(1, p_columns);
	if (slot_grid) {
		slot_grid->set_columns(columns);
	}
}

void ContainerPanel::set_slot_size(const Size2 &p_size) {
	slot_size = p_size;
	for (ItemSlot *slot : slots) {
		slot->set_custom_minimum_size(slot_size);
	}
}

void ContainerPanel::set_slot_separation(int32_t p_sep) {
	slot_separation = p_sep;
	if (slot_grid) {
		slot_grid->add_theme_constant_override("h_separation", slot_separation);
		slot_grid->add_theme_constant_override("v_separation", slot_separation);
	}
}

void ContainerPanel::set_title(const String &p_title) {
	if (title_label) {
		title_label->set_text(p_title);
	}
}

String ContainerPanel::get_title() const {
	return title_label ? title_label->get_text() : String();
}

ItemSlot *ContainerPanel::get_slot(int32_t p_index) const {
	ERR_FAIL_INDEX_V(p_index, slots.size(), nullptr);
	return slots[p_index];
}

void ContainerPanel::_rebuild_slots() {
	if (!slot_grid || !bound_object) {
		return;
	}

	int32_t capacity = bound_object->get_container_capacity();

	// 移除多余的槽位
	while (slots.size() > capacity) {
		ItemSlot *slot = slots[slots.size() - 1];
		slots.remove_at(slots.size() - 1);
		slot_grid->remove_child(slot);
		memdelete(slot);
	}

	// 添加不足的槽位
	while (slots.size() < capacity) {
		ItemSlot *slot = memnew(ItemSlot);
		slot->set_custom_minimum_size(slot_size);
		slot->set_slot_index(slots.size());

		// 连接信号
		int slot_index = slots.size();
		slot->connect("clicked", callable_mp(this, &ContainerPanel::_on_slot_clicked).bind(slot_index));
		slot->connect("item_dropped", callable_mp(this, &ContainerPanel::_on_slot_item_dropped).bind(slot_index));

		slot_grid->add_child(slot);
		slots.push_back(slot);
	}
}

void ContainerPanel::_sync_from_container() {
	if (!bound_object) {
		return;
	}

	// 直接从容器同步物品到UI
	for (int i = 0; i < slots.size(); i++) {
		Ref<Item> item = bound_object->container_get_item(i);
		slots[i]->set_item(item);
	}
}

void ContainerPanel::_on_slot_clicked(int button_index, int slot_index) {
	emit_signal("slot_clicked", slot_index, button_index);

	// 右键使用物品
	if (button_index == 2) {  // MOUSE_BUTTON_RIGHT
		ItemSlot *slot = get_slot(slot_index);
		if (slot && slot->has_item()) {
			emit_signal("item_used", slot_index, slot->get_item());
		}
	}
}

void ContainerPanel::_on_slot_item_dropped(ItemSlot *from_slot, Ref<Item> item, int to_slot_index) {
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
		emit_signal("item_moved", from_index, to_slot_index);
	}
}
