/**************************************************************************/
/*  player_hud.cpp                                                        */
/**************************************************************************/

#include "player_hud.h"

#include "../player.h"
#include "scene/gui/box_container.h"
#include "scene/gui/margin_container.h"
#include "scene/gui/panel_container.h"
#include "scene/resources/style_box_flat.h"

void PlayerHUD::_bind_methods() {
	ClassDB::bind_method(D_METHOD("bind_player", "player"), &PlayerHUD::bind_player);
	ClassDB::bind_method(D_METHOD("unbind_player"), &PlayerHUD::unbind_player);
	ClassDB::bind_method(D_METHOD("get_bound_player"), &PlayerHUD::get_bound_player);

	ClassDB::bind_method(D_METHOD("refresh"), &PlayerHUD::refresh);

	ClassDB::bind_method(D_METHOD("set_show_health", "show"), &PlayerHUD::set_show_health);
	ClassDB::bind_method(D_METHOD("is_show_health"), &PlayerHUD::is_show_health);

	ClassDB::bind_method(D_METHOD("set_show_mana", "show"), &PlayerHUD::set_show_mana);
	ClassDB::bind_method(D_METHOD("is_show_mana"), &PlayerHUD::is_show_mana);

	ClassDB::bind_method(D_METHOD("set_show_exp", "show"), &PlayerHUD::set_show_exp);
	ClassDB::bind_method(D_METHOD("is_show_exp"), &PlayerHUD::is_show_exp);

	ClassDB::bind_method(D_METHOD("set_show_gold", "show"), &PlayerHUD::set_show_gold);
	ClassDB::bind_method(D_METHOD("is_show_gold"), &PlayerHUD::is_show_gold);

	ClassDB::bind_method(D_METHOD("set_show_level", "show"), &PlayerHUD::set_show_level);
	ClassDB::bind_method(D_METHOD("is_show_level"), &PlayerHUD::is_show_level);

	ClassDB::bind_method(D_METHOD("set_show_numeric_values", "show"), &PlayerHUD::set_show_numeric_values);
	ClassDB::bind_method(D_METHOD("is_show_numeric_values"), &PlayerHUD::is_show_numeric_values);

	ClassDB::bind_method(D_METHOD("set_health_color", "color"), &PlayerHUD::set_health_color);
	ClassDB::bind_method(D_METHOD("get_health_color"), &PlayerHUD::get_health_color);

	ClassDB::bind_method(D_METHOD("set_mana_color", "color"), &PlayerHUD::set_mana_color);
	ClassDB::bind_method(D_METHOD("get_mana_color"), &PlayerHUD::get_mana_color);

	ClassDB::bind_method(D_METHOD("set_exp_color", "color"), &PlayerHUD::set_exp_color);
	ClassDB::bind_method(D_METHOD("get_exp_color"), &PlayerHUD::get_exp_color);

	// Properties
	ADD_GROUP("Display", "show_");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "show_health"), "set_show_health", "is_show_health");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "show_mana"), "set_show_mana", "is_show_mana");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "show_exp"), "set_show_exp", "is_show_exp");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "show_gold"), "set_show_gold", "is_show_gold");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "show_level"), "set_show_level", "is_show_level");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "show_numeric_values"), "set_show_numeric_values", "is_show_numeric_values");

	ADD_GROUP("Colors", "");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "health_color"), "set_health_color", "get_health_color");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "mana_color"), "set_mana_color", "get_mana_color");
	ADD_PROPERTY(PropertyInfo(Variant::COLOR, "exp_color"), "set_exp_color", "get_exp_color");

	// Signals
	ADD_SIGNAL(MethodInfo("player_bound", PropertyInfo(Variant::OBJECT, "player")));
	ADD_SIGNAL(MethodInfo("player_unbound"));
}

void PlayerHUD::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
			_build_ui();
		} break;

		case NOTIFICATION_PROCESS: {
			if (bound_player) {
				_update_all();
			}
		} break;
	}
}

PlayerHUD::PlayerHUD() {
	set_mouse_filter(MOUSE_FILTER_IGNORE);
	set_process(false);
}

PlayerHUD::~PlayerHUD() {
	unbind_player();
}

void PlayerHUD::_build_ui() {
	// Main container - top left layout
	VBoxContainer *main_vbox = memnew(VBoxContainer);
	main_vbox->set_anchors_preset(PRESET_TOP_LEFT);
	main_vbox->set_offset(SIDE_LEFT, 20);
	main_vbox->set_offset(SIDE_TOP, 20);
	main_vbox->add_theme_constant_override("separation", 8);
	add_child(main_vbox);

	// === Level and Gold info row ===
	info_container = memnew(HBoxContainer);
	Object::cast_to<HBoxContainer>(info_container)->add_theme_constant_override("separation", 20);
	main_vbox->add_child(info_container);

	// Level label
	level_label = memnew(Label);
	level_label->set_text("Lv. 1");
	level_label->add_theme_font_size_override("font_size", 20);
	level_label->add_theme_color_override("font_color", Color(1.0, 0.85, 0.4, 1.0));
	info_container->add_child(level_label);

	// Gold label
	HBoxContainer *gold_box = memnew(HBoxContainer);
	gold_box->add_theme_constant_override("separation", 4);
	info_container->add_child(gold_box);

	Label *gold_icon = memnew(Label);
	gold_icon->set_text("G");  // Gold icon
	gold_icon->add_theme_color_override("font_color", Color(1.0, 0.85, 0.2, 1.0));
	gold_box->add_child(gold_icon);

	gold_label = memnew(Label);
	gold_label->set_text("0");
	gold_label->add_theme_font_size_override("font_size", 18);
	gold_label->add_theme_color_override("font_color", Color(1.0, 0.85, 0.2, 1.0));
	gold_box->add_child(gold_label);

	// === Health bar ===
	health_container = memnew(HBoxContainer);
	Object::cast_to<HBoxContainer>(health_container)->add_theme_constant_override("separation", 8);
	main_vbox->add_child(health_container);

	Label *health_icon = memnew(Label);
	health_icon->set_text("HP");
	health_icon->add_theme_color_override("font_color", health_color);
	health_container->add_child(health_icon);

	health_bar = memnew(ProgressBar);
	health_bar->set_custom_minimum_size(Size2(200, 20));
	health_bar->set_max(100);
	health_bar->set_value(100);
	health_bar->set_show_percentage(false);
	health_bar->add_theme_color_override("font_color", Color(1, 1, 1, 1));

	Ref<StyleBoxFlat> health_fill;
	health_fill.instantiate();
	health_fill->set_bg_color(health_color);
	health_fill->set_corner_radius_all(4);
	health_bar->add_theme_style_override("fill", health_fill);

	Ref<StyleBoxFlat> health_bg;
	health_bg.instantiate();
	health_bg->set_bg_color(Color(0.15, 0.15, 0.15, 0.9));
	health_bg->set_corner_radius_all(4);
	health_bar->add_theme_style_override("background", health_bg);
	health_container->add_child(health_bar);

	health_label = memnew(Label);
	health_label->set_text("100/100");
	health_label->add_theme_font_size_override("font_size", 14);
	health_container->add_child(health_label);

	// === Mana bar ===
	mana_container = memnew(HBoxContainer);
	Object::cast_to<HBoxContainer>(mana_container)->add_theme_constant_override("separation", 8);
	main_vbox->add_child(mana_container);

	Label *mana_icon = memnew(Label);
	mana_icon->set_text("MP");
	mana_icon->add_theme_color_override("font_color", mana_color);
	mana_container->add_child(mana_icon);

	mana_bar = memnew(ProgressBar);
	mana_bar->set_custom_minimum_size(Size2(200, 16));
	mana_bar->set_max(100);
	mana_bar->set_value(100);
	mana_bar->set_show_percentage(false);

	Ref<StyleBoxFlat> mana_fill;
	mana_fill.instantiate();
	mana_fill->set_bg_color(mana_color);
	mana_fill->set_corner_radius_all(4);
	mana_bar->add_theme_style_override("fill", mana_fill);

	Ref<StyleBoxFlat> mana_bg;
	mana_bg.instantiate();
	mana_bg->set_bg_color(Color(0.15, 0.15, 0.15, 0.9));
	mana_bg->set_corner_radius_all(4);
	mana_bar->add_theme_style_override("background", mana_bg);
	mana_container->add_child(mana_bar);

	mana_label = memnew(Label);
	mana_label->set_text("100/100");
	mana_label->add_theme_font_size_override("font_size", 14);
	mana_container->add_child(mana_label);

	// === Exp bar ===
	exp_container = memnew(HBoxContainer);
	Object::cast_to<HBoxContainer>(exp_container)->add_theme_constant_override("separation", 8);
	main_vbox->add_child(exp_container);

	Label *exp_icon = memnew(Label);
	exp_icon->set_text("EXP");
	exp_icon->add_theme_color_override("font_color", exp_color);
	exp_container->add_child(exp_icon);

	exp_bar = memnew(ProgressBar);
	exp_bar->set_custom_minimum_size(Size2(200, 12));
	exp_bar->set_max(100);
	exp_bar->set_value(0);
	exp_bar->set_show_percentage(false);

	Ref<StyleBoxFlat> exp_fill;
	exp_fill.instantiate();
	exp_fill->set_bg_color(exp_color);
	exp_fill->set_corner_radius_all(4);
	exp_bar->add_theme_style_override("fill", exp_fill);

	Ref<StyleBoxFlat> exp_bg;
	exp_bg.instantiate();
	exp_bg->set_bg_color(Color(0.15, 0.15, 0.15, 0.9));
	exp_bg->set_corner_radius_all(4);
	exp_bar->add_theme_style_override("background", exp_bg);
	exp_container->add_child(exp_bar);

	exp_label = memnew(Label);
	exp_label->set_text("0/100");
	exp_label->add_theme_font_size_override("font_size", 12);
	exp_container->add_child(exp_label);

	// Apply display settings
	health_container->set_visible(show_health);
	mana_container->set_visible(show_mana);
	exp_container->set_visible(show_exp);
	if (Control *gold_parent = Object::cast_to<Control>(gold_label->get_parent())) {
		gold_parent->set_visible(show_gold);
	}
	level_label->set_visible(show_level);
	health_label->set_visible(show_numeric_values);
	mana_label->set_visible(show_numeric_values);
	exp_label->set_visible(show_numeric_values);
}

void PlayerHUD::bind_player(Player *p_player) {
	if (bound_player == p_player) {
		return;
	}

	unbind_player();
	bound_player = p_player;

	if (bound_player) {
		_update_all();
		set_process(true);
		emit_signal("player_bound", bound_player);
	}
}

void PlayerHUD::unbind_player() {
	if (bound_player) {
		bound_player = nullptr;
		set_process(false);
		emit_signal("player_unbound");
	}
}

void PlayerHUD::refresh() {
	_update_all();
}

void PlayerHUD::_update_health() {
	if (!bound_player || !health_bar) {
		return;
	}

	float current = bound_player->get_health();
	float max_val = bound_player->get_max_health();

	health_bar->set_max(max_val);
	health_bar->set_value(current);

	if (health_label && show_numeric_values) {
		health_label->set_text(vformat("%d/%d", (int)current, (int)max_val));
	}
}

void PlayerHUD::_update_mana() {
	if (!bound_player || !mana_bar) {
		return;
	}

	float current = bound_player->get_mana();
	float max_val = bound_player->get_max_mana();

	mana_bar->set_max(max_val);
	mana_bar->set_value(current);

	if (mana_label && show_numeric_values) {
		mana_label->set_text(vformat("%d/%d", (int)current, (int)max_val));
	}
}

void PlayerHUD::_update_exp() {
	if (!bound_player || !exp_bar) {
		return;
	}

	int64_t current = bound_player->get_experience();
	int64_t max_val = bound_player->get_experience_to_next_level();

	exp_bar->set_max((double)max_val);
	exp_bar->set_value((double)current);

	if (exp_label && show_numeric_values) {
		exp_label->set_text(vformat("%d/%d", current, max_val));
	}
}

void PlayerHUD::_update_gold() {
	if (!bound_player || !gold_label) {
		return;
	}

	int64_t gold = bound_player->get_gold();

	// Format large numbers
	if (gold >= 1000000) {
		gold_label->set_text(vformat("%.1fM", gold / 1000000.0));
	} else if (gold >= 1000) {
		gold_label->set_text(vformat("%.1fK", gold / 1000.0));
	} else {
		gold_label->set_text(String::num_int64(gold));
	}
}

void PlayerHUD::_update_level() {
	if (!bound_player || !level_label) {
		return;
	}

	level_label->set_text(vformat("Lv. %d", bound_player->get_level()));
}

void PlayerHUD::_update_all() {
	_update_health();
	_update_mana();
	_update_exp();
	_update_gold();
	_update_level();
}

void PlayerHUD::_on_player_health_changed() {
	_update_health();
}

void PlayerHUD::_on_player_mana_changed() {
	_update_mana();
}

void PlayerHUD::_on_player_level_changed(int32_t new_level) {
	_update_level();
	_update_exp();
}

void PlayerHUD::_on_player_gold_changed(int64_t old_amount, int64_t new_amount) {
	_update_gold();
}

void PlayerHUD::_on_player_exp_gained(int64_t amount) {
	_update_exp();
}

// === Display config ===

void PlayerHUD::set_show_health(bool p_show) {
	show_health = p_show;
	if (health_container) {
		health_container->set_visible(p_show);
	}
}

void PlayerHUD::set_show_mana(bool p_show) {
	show_mana = p_show;
	if (mana_container) {
		mana_container->set_visible(p_show);
	}
}

void PlayerHUD::set_show_exp(bool p_show) {
	show_exp = p_show;
	if (exp_container) {
		exp_container->set_visible(p_show);
	}
}

void PlayerHUD::set_show_gold(bool p_show) {
	show_gold = p_show;
	if (gold_label && gold_label->get_parent()) {
		if (Control *gold_parent = Object::cast_to<Control>(gold_label->get_parent())) {
			gold_parent->set_visible(p_show);
		}
	}
}

void PlayerHUD::set_show_level(bool p_show) {
	show_level = p_show;
	if (level_label) {
		level_label->set_visible(p_show);
	}
}

void PlayerHUD::set_show_numeric_values(bool p_show) {
	show_numeric_values = p_show;
	if (health_label) {
		health_label->set_visible(p_show);
	}
	if (mana_label) {
		mana_label->set_visible(p_show);
	}
	if (exp_label) {
		exp_label->set_visible(p_show);
	}
}

// === Style ===

void PlayerHUD::set_health_color(const Color &p_color) {
	health_color = p_color;
	if (health_bar) {
		Ref<StyleBoxFlat> fill = health_bar->get_theme_stylebox("fill");
		if (fill.is_valid()) {
			fill->set_bg_color(p_color);
		}
	}
}

void PlayerHUD::set_mana_color(const Color &p_color) {
	mana_color = p_color;
	if (mana_bar) {
		Ref<StyleBoxFlat> fill = mana_bar->get_theme_stylebox("fill");
		if (fill.is_valid()) {
			fill->set_bg_color(p_color);
		}
	}
}

void PlayerHUD::set_exp_color(const Color &p_color) {
	exp_color = p_color;
	if (exp_bar) {
		Ref<StyleBoxFlat> fill = exp_bar->get_theme_stylebox("fill");
		if (fill.is_valid()) {
			fill->set_bg_color(p_color);
		}
	}
}
