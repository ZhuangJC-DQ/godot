/**************************************************************************/
/*  item_slot.cpp                                                         */
/**************************************************************************/

#include "item_slot.h"

#include "../item.h"
#include "scene/gui/panel.h"

void ItemSlot::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_item", "item"), &ItemSlot::set_item);
	ClassDB::bind_method(D_METHOD("get_item"), &ItemSlot::get_item);
	ClassDB::bind_method(D_METHOD("has_item"), &ItemSlot::has_item);
	ClassDB::bind_method(D_METHOD("clear_item"), &ItemSlot::clear_item);

	ClassDB::bind_method(D_METHOD("set_slot_index", "index"), &ItemSlot::set_slot_index);
	ClassDB::bind_method(D_METHOD("get_slot_index"), &ItemSlot::get_slot_index);

	ClassDB::bind_method(D_METHOD("set_slot_state", "state"), &ItemSlot::set_slot_state);
	ClassDB::bind_method(D_METHOD("get_slot_state"), &ItemSlot::get_slot_state);

	ClassDB::bind_method(D_METHOD("set_draggable", "draggable"), &ItemSlot::set_draggable);
	ClassDB::bind_method(D_METHOD("is_draggable"), &ItemSlot::is_draggable);

	ClassDB::bind_method(D_METHOD("set_droppable", "droppable"), &ItemSlot::set_droppable);
	ClassDB::bind_method(D_METHOD("is_droppable"), &ItemSlot::is_droppable);

	ClassDB::bind_method(D_METHOD("set_empty_texture", "texture"), &ItemSlot::set_empty_texture);
	ClassDB::bind_method(D_METHOD("get_empty_texture"), &ItemSlot::get_empty_texture);

	// 属性
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "item", PROPERTY_HINT_RESOURCE_TYPE, "Item"), "set_item", "get_item");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "slot_index"), "set_slot_index", "get_slot_index");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "slot_state", PROPERTY_HINT_ENUM, "Normal,Hovered,Pressed,Disabled,Locked"), "set_slot_state", "get_slot_state");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "draggable"), "set_draggable", "is_draggable");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "droppable"), "set_droppable", "is_droppable");
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "empty_texture", PROPERTY_HINT_RESOURCE_TYPE, "Texture2D"), "set_empty_texture", "get_empty_texture");

	// 信号
	ADD_SIGNAL(MethodInfo("item_changed", PropertyInfo(Variant::OBJECT, "new_item", PROPERTY_HINT_RESOURCE_TYPE, "Item")));
	ADD_SIGNAL(MethodInfo("clicked", PropertyInfo(Variant::INT, "button_index")));
	ADD_SIGNAL(MethodInfo("double_clicked"));
	ADD_SIGNAL(MethodInfo("hovered"));
	ADD_SIGNAL(MethodInfo("unhovered"));
	ADD_SIGNAL(MethodInfo("item_dropped", PropertyInfo(Variant::OBJECT, "from_slot"), PropertyInfo(Variant::OBJECT, "item")));

	// 枚举
	BIND_ENUM_CONSTANT(STATE_NORMAL);
	BIND_ENUM_CONSTANT(STATE_HOVERED);
	BIND_ENUM_CONSTANT(STATE_PRESSED);
	BIND_ENUM_CONSTANT(STATE_DISABLED);
	BIND_ENUM_CONSTANT(STATE_LOCKED);
}

void ItemSlot::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
			// 创建子控件
			if (!icon) {
				icon = memnew(TextureRect);
				icon->set_anchors_and_offsets_preset(PRESET_FULL_RECT);
				icon->set_expand_mode(TextureRect::EXPAND_IGNORE_SIZE);
				icon->set_stretch_mode(TextureRect::STRETCH_KEEP_ASPECT_CENTERED);
				icon->set_mouse_filter(MOUSE_FILTER_IGNORE);
				add_child(icon);
			}

			if (!quantity_label) {
				quantity_label = memnew(Label);
				quantity_label->set_anchors_and_offsets_preset(PRESET_BOTTOM_RIGHT);
				quantity_label->set_horizontal_alignment(HORIZONTAL_ALIGNMENT_RIGHT);
				quantity_label->set_vertical_alignment(VERTICAL_ALIGNMENT_BOTTOM);
				quantity_label->set_mouse_filter(MOUSE_FILTER_IGNORE);
				add_child(quantity_label);
			}

			if (!highlight) {
				highlight = memnew(Control);
				highlight->set_anchors_and_offsets_preset(PRESET_FULL_RECT);
				highlight->set_mouse_filter(MOUSE_FILTER_IGNORE);
				highlight->hide();
				add_child(highlight);
			}

			_update_display();
		} break;

		case NOTIFICATION_MOUSE_ENTER: {
			if (state != STATE_DISABLED && state != STATE_LOCKED) {
				set_slot_state(STATE_HOVERED);
				emit_signal("hovered");
			}
		} break;

		case NOTIFICATION_MOUSE_EXIT: {
			if (state == STATE_HOVERED) {
				set_slot_state(STATE_NORMAL);
				emit_signal("unhovered");
			}
		} break;

		case NOTIFICATION_DRAW: {
			// 绘制背景边框
			Rect2 rect = Rect2(Vector2(), get_size());
			Color border_color = Color(0.3, 0.3, 0.3, 1.0);
			Color bg_color = Color(0.1, 0.1, 0.1, 0.8);

			switch (state) {
				case STATE_HOVERED:
					border_color = Color(0.8, 0.8, 0.5, 1.0);
					bg_color = Color(0.15, 0.15, 0.15, 0.9);
					break;
				case STATE_PRESSED:
					border_color = Color(1.0, 1.0, 0.6, 1.0);
					bg_color = Color(0.2, 0.2, 0.1, 0.9);
					break;
				case STATE_DISABLED:
					border_color = Color(0.2, 0.2, 0.2, 0.5);
					bg_color = Color(0.05, 0.05, 0.05, 0.5);
					break;
				case STATE_LOCKED:
					border_color = Color(0.5, 0.2, 0.2, 1.0);
					bg_color = Color(0.1, 0.05, 0.05, 0.8);
					break;
				default:
					break;
			}

			// 如果有物品，根据稀有度调整边框颜色
			if (item.is_valid() && state == STATE_NORMAL) {
				border_color = get_rarity_color(item->get_rarity());
			}

			draw_rect(rect, bg_color);
			draw_rect(rect, border_color, false, 2.0);
		} break;
	}
}

ItemSlot::ItemSlot() {
	set_custom_minimum_size(Size2(64, 64));
	set_mouse_filter(MOUSE_FILTER_STOP);

	// 初始化稀有度颜色
	rarity_colors[0] = Color(0.6, 0.6, 0.6, 1.0);   // 普通 - 灰色
	rarity_colors[1] = Color(0.3, 0.8, 0.3, 1.0);   // 非凡 - 绿色
	rarity_colors[2] = Color(0.3, 0.5, 1.0, 1.0);   // 稀有 - 蓝色
	rarity_colors[3] = Color(0.7, 0.3, 0.9, 1.0);   // 史诗 - 紫色
	rarity_colors[4] = Color(1.0, 0.6, 0.2, 1.0);   // 传说 - 橙色
	rarity_colors[5] = Color(1.0, 1.0, 1.0, 1.0);   // 备用
}

ItemSlot::~ItemSlot() {
}

void ItemSlot::set_item(const Ref<Item> &p_item) {
	if (item == p_item) {
		return;
	}

	item = p_item;
	_update_display();
	emit_signal("item_changed", item);
}

void ItemSlot::clear_item() {
	set_item(Ref<Item>());
}

void ItemSlot::set_slot_state(SlotState p_state) {
	if (state == p_state) {
		return;
	}

	state = p_state;
	queue_redraw();
}

void ItemSlot::set_empty_texture(const Ref<Texture2D> &p_texture) {
	empty_texture = p_texture;
	_update_display();
}

void ItemSlot::set_rarity_color(int p_rarity, const Color &p_color) {
	ERR_FAIL_INDEX(p_rarity, 6);
	rarity_colors[p_rarity] = p_color;
	queue_redraw();
}

Color ItemSlot::get_rarity_color(int p_rarity) const {
	ERR_FAIL_INDEX_V(p_rarity, 6, Color());
	return rarity_colors[p_rarity];
}

void ItemSlot::_update_display() {
	if (!icon || !quantity_label) {
		return;
	}

	if (item.is_valid()) {
		// TODO: 从资源管理器获取物品图标
		// icon->set_texture(ItemDatabase::get_icon(item->get_item_id()));

		// 显示数量（堆叠物品）
		if (item->get_quantity() > 1) {
			quantity_label->set_text(String::num_int64(item->get_quantity()));
			quantity_label->show();
		} else {
			quantity_label->hide();
		}
	} else {
		icon->set_texture(empty_texture);
		quantity_label->hide();
	}

	queue_redraw();
}

void ItemSlot::gui_input(const Ref<InputEvent> &p_event) {
	if (state == STATE_DISABLED || state == STATE_LOCKED) {
		return;
	}

	Ref<InputEventMouseButton> mb = p_event;
	if (mb.is_valid()) {
		if (mb->is_pressed()) {
			set_slot_state(STATE_PRESSED);

			if (mb->is_double_click()) {
				emit_signal("double_clicked");
			}
		} else {
			set_slot_state(STATE_HOVERED);
			emit_signal("clicked", (int)mb->get_button_index());
		}
	}
}

Variant ItemSlot::get_drag_data(const Point2 &p_point) {
	if (!draggable || !item.is_valid()) {
		return Variant();
	}

	// 创建拖拽预览
	ItemSlot *preview = memnew(ItemSlot);
	preview->set_item(item);
	preview->set_custom_minimum_size(get_size());
	preview->set_modulate(Color(1, 1, 1, 0.7));
	set_drag_preview(preview);

	// 返回拖拽数据
	Dictionary data;
	data["type"] = "item_slot";
	data["item"] = item;
	data["source_slot"] = this;
	data["slot_index"] = slot_index;

	return data;
}

bool ItemSlot::can_drop_data(const Point2 &p_point, const Variant &p_data) const {
	if (!droppable) {
		return false;
	}

	if (p_data.get_type() != Variant::DICTIONARY) {
		return false;
	}

	Dictionary data = p_data;
	return data.has("type") && String(data["type"]) == "item_slot";
}

void ItemSlot::drop_data(const Point2 &p_point, const Variant &p_data) {
	Dictionary data = p_data;
	ItemSlot *source_slot = Object::cast_to<ItemSlot>(data["source_slot"]);
	Ref<Item> dropped_item = data["item"];

	if (source_slot && dropped_item.is_valid()) {
		emit_signal("item_dropped", source_slot, dropped_item);
	}
}
