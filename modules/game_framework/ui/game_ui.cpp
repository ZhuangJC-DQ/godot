/**************************************************************************/
/*  game_ui.cpp                                                           */
/**************************************************************************/

#include "game_ui.h"

#include "../item.h"
#include "../player.h"
#include "scene/main/viewport.h"

void GameUI::_bind_methods() {
	ClassDB::bind_method(D_METHOD("setup"), &GameUI::setup);

	ClassDB::bind_method(D_METHOD("bind_player", "player"), &GameUI::bind_player);
	ClassDB::bind_method(D_METHOD("unbind_player"), &GameUI::unbind_player);
	ClassDB::bind_method(D_METHOD("get_bound_player"), &GameUI::get_bound_player);

	ClassDB::bind_method(D_METHOD("get_layer", "layer"), &GameUI::get_layer);

	ClassDB::bind_method(D_METHOD("show_panel", "panel"), &GameUI::show_panel);
	ClassDB::bind_method(D_METHOD("hide_panel", "panel"), &GameUI::hide_panel);
	ClassDB::bind_method(D_METHOD("hide_all_panels"), &GameUI::hide_all_panels);
	ClassDB::bind_method(D_METHOD("is_any_panel_open"), &GameUI::is_any_panel_open);

	ClassDB::bind_method(D_METHOD("show_tooltip", "text", "position"), &GameUI::show_tooltip);
	ClassDB::bind_method(D_METHOD("show_item_tooltip", "item", "position"), &GameUI::show_item_tooltip);
	ClassDB::bind_method(D_METHOD("hide_tooltip"), &GameUI::hide_tooltip);

	// 信号
	ADD_SIGNAL(MethodInfo("player_bound", PropertyInfo(Variant::OBJECT, "player")));
	ADD_SIGNAL(MethodInfo("player_unbound"));
	ADD_SIGNAL(MethodInfo("panel_opened", PropertyInfo(Variant::OBJECT, "panel")));
	ADD_SIGNAL(MethodInfo("panel_closed", PropertyInfo(Variant::OBJECT, "panel")));

	// 枚举
	BIND_ENUM_CONSTANT(LAYER_HUD);
	BIND_ENUM_CONSTANT(LAYER_PANEL);
	BIND_ENUM_CONSTANT(LAYER_POPUP);
	BIND_ENUM_CONSTANT(LAYER_TOOLTIP);
}

void GameUI::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
			setup();
		} break;
	}
}

GameUI::GameUI() {
	set_anchors_and_offsets_preset(PRESET_FULL_RECT);
	set_mouse_filter(MOUSE_FILTER_IGNORE);
}

GameUI::~GameUI() {
	unbind_player();
}

void GameUI::setup() {
	// 创建UI层级
	const char *layer_names[] = { "HUDLayer", "PanelLayer", "PopupLayer", "TooltipLayer" };

	for (int i = 0; i < LAYER_MAX; i++) {
		if (layers[i] == nullptr) {
			layers[i] = memnew(Control);
			layers[i]->set_name(layer_names[i]);
			layers[i]->set_anchors_and_offsets_preset(PRESET_FULL_RECT);
			layers[i]->set_mouse_filter(MOUSE_FILTER_IGNORE);
			add_child(layers[i]);
		}
	}
}

void GameUI::bind_player(Player *p_player) {
	if (bound_player == p_player) {
		return;
	}

	unbind_player();
	bound_player = p_player;

	if (bound_player) {
		emit_signal("player_bound", bound_player);
	}
}

void GameUI::unbind_player() {
	if (bound_player) {
		bound_player = nullptr;
		emit_signal("player_unbound");
	}
}

Control *GameUI::get_layer(Layer p_layer) const {
	ERR_FAIL_INDEX_V(p_layer, LAYER_MAX, nullptr);
	return layers[p_layer];
}

void GameUI::show_panel(Control *p_panel) {
	ERR_FAIL_NULL(p_panel);

	if (p_panel->get_parent() != layers[LAYER_PANEL]) {
		if (p_panel->get_parent()) {
			p_panel->get_parent()->remove_child(p_panel);
		}
		layers[LAYER_PANEL]->add_child(p_panel);
	}

	p_panel->show();
	panel_open = true;
	emit_signal("panel_opened", p_panel);
}

void GameUI::hide_panel(Control *p_panel) {
	ERR_FAIL_NULL(p_panel);

	p_panel->hide();
	emit_signal("panel_closed", p_panel);

	// 检查是否还有其他面板打开
	panel_open = false;
	for (int i = 0; i < layers[LAYER_PANEL]->get_child_count(); i++) {
		Control *child = Object::cast_to<Control>(layers[LAYER_PANEL]->get_child(i));
		if (child && child->is_visible()) {
			panel_open = true;
			break;
		}
	}
}

void GameUI::hide_all_panels() {
	for (int i = 0; i < layers[LAYER_PANEL]->get_child_count(); i++) {
		Control *child = Object::cast_to<Control>(layers[LAYER_PANEL]->get_child(i));
		if (child && child->is_visible()) {
			child->hide();
			emit_signal("panel_closed", child);
		}
	}
	panel_open = false;
}

void GameUI::show_tooltip(const String &p_text, const Vector2 &p_position) {
	// TODO: 实现通用tooltip
}

void GameUI::show_item_tooltip(const Ref<Item> &p_item, const Vector2 &p_position) {
	// TODO: 实现物品tooltip
}

void GameUI::hide_tooltip() {
	// TODO: 隐藏tooltip
}

void GameUI::gui_input(const Ref<InputEvent> &p_event) {
	// ESC关闭所有面板
	Ref<InputEventKey> key = p_event;
	if (key.is_valid() && key->is_pressed() && !key->is_echo()) {
		if (key->get_keycode() == Key::ESCAPE && panel_open) {
			hide_all_panels();
			accept_event();
		}
	}
}
