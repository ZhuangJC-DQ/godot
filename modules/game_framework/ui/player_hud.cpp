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

	ADD_GROUP("Node Paths", "");
	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "health_bar_path"), "set_health_bar_path", "get_health_bar_path");
	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "health_label_path"), "set_health_label_path", "get_health_label_path");
	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "mana_bar_path"), "set_mana_bar_path", "get_mana_bar_path");
	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "mana_label_path"), "set_mana_label_path", "get_mana_label_path");
	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "exp_bar_path"), "set_exp_bar_path", "get_exp_bar_path");
	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "exp_label_path"), "set_exp_label_path", "get_exp_label_path");
	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "gold_label_path"), "set_gold_label_path", "get_gold_label_path");
	ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "level_label_path"), "set_level_label_path", "get_level_label_path");

	// NodePath accessors
	ClassDB::bind_method(D_METHOD("set_health_bar_path", "path"), &PlayerHUD::set_health_bar_path);
	ClassDB::bind_method(D_METHOD("get_health_bar_path"), &PlayerHUD::get_health_bar_path);

	ClassDB::bind_method(D_METHOD("set_health_label_path", "path"), &PlayerHUD::set_health_label_path);
	ClassDB::bind_method(D_METHOD("get_health_label_path"), &PlayerHUD::get_health_label_path);

	ClassDB::bind_method(D_METHOD("set_mana_bar_path", "path"), &PlayerHUD::set_mana_bar_path);
	ClassDB::bind_method(D_METHOD("get_mana_bar_path"), &PlayerHUD::get_mana_bar_path);

	ClassDB::bind_method(D_METHOD("set_mana_label_path", "path"), &PlayerHUD::set_mana_label_path);
	ClassDB::bind_method(D_METHOD("get_mana_label_path"), &PlayerHUD::get_mana_label_path);

	ClassDB::bind_method(D_METHOD("set_exp_bar_path", "path"), &PlayerHUD::set_exp_bar_path);
	ClassDB::bind_method(D_METHOD("get_exp_bar_path"), &PlayerHUD::get_exp_bar_path);

	ClassDB::bind_method(D_METHOD("set_exp_label_path", "path"), &PlayerHUD::set_exp_label_path);
	ClassDB::bind_method(D_METHOD("get_exp_label_path"), &PlayerHUD::get_exp_label_path);

	ClassDB::bind_method(D_METHOD("set_gold_label_path", "path"), &PlayerHUD::set_gold_label_path);
	ClassDB::bind_method(D_METHOD("get_gold_label_path"), &PlayerHUD::get_gold_label_path);

	ClassDB::bind_method(D_METHOD("set_level_label_path", "path"), &PlayerHUD::set_level_label_path);
	ClassDB::bind_method(D_METHOD("get_level_label_path"), &PlayerHUD::get_level_label_path);

	// Signals
	ADD_SIGNAL(MethodInfo("player_bound", PropertyInfo(Variant::OBJECT, "player")));
	ADD_SIGNAL(MethodInfo("player_unbound"));
}

void PlayerHUD::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
			_get_ui_nodes();
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

void PlayerHUD::_get_ui_nodes() {
	// 通过NodePath获取场景中的UI节点
	// 使用%唯一名称可以避免完整路径依赖

	// 获取容器节点
	info_container = Object::cast_to<Control>(get_node_or_null(info_container_path));
	health_container = Object::cast_to<Control>(get_node_or_null(health_container_path));
	mana_container = Object::cast_to<Control>(get_node_or_null(mana_container_path));
	exp_container = Object::cast_to<Control>(get_node_or_null(exp_container_path));

	// 获取进度条节点
	health_bar = Object::cast_to<ProgressBar>(get_node_or_null(health_bar_path));
	mana_bar = Object::cast_to<ProgressBar>(get_node_or_null(mana_bar_path));
	exp_bar = Object::cast_to<ProgressBar>(get_node_or_null(exp_bar_path));

	// 获取标签节点
	health_label = Object::cast_to<Label>(get_node_or_null(health_label_path));
	mana_label = Object::cast_to<Label>(get_node_or_null(mana_label_path));
	exp_label = Object::cast_to<Label>(get_node_or_null(exp_label_path));
	gold_label = Object::cast_to<Label>(get_node_or_null(gold_label_path));
	level_label = Object::cast_to<Label>(get_node_or_null(level_label_path));

	// 检查必需的节点是否存在
	if (!health_bar) {
		ERR_PRINT(vformat("PlayerHUD: 无法找到HealthBar节点，路径: %s。请从场景文件实例化PlayerHUD。", health_bar_path));
	}
	if (!mana_bar) {
		ERR_PRINT(vformat("PlayerHUD: 无法找到ManaBar节点，路径: %s。请从场景文件实例化PlayerHUD。", mana_bar_path));
	}
	if (!exp_bar) {
		ERR_PRINT(vformat("PlayerHUD: 无法找到ExpBar节点，路径: %s。请从场景文件实例化PlayerHUD。", exp_bar_path));
	}

	// 应用显示设置
	if (health_container) {
		health_container->set_visible(show_health);
	}
	if (mana_container) {
		mana_container->set_visible(show_mana);
	}
	if (exp_container) {
		exp_container->set_visible(show_exp);
	}
	if (gold_label && gold_label->get_parent()) {
		if (Control *gold_parent = Object::cast_to<Control>(gold_label->get_parent())) {
			gold_parent->set_visible(show_gold);
		}
	}
	if (level_label) {
		level_label->set_visible(show_level);
	}
	if (health_label) {
		health_label->set_visible(show_numeric_values);
	}
	if (mana_label) {
		mana_label->set_visible(show_numeric_values);
	}
	if (exp_label) {
		exp_label->set_visible(show_numeric_values);
	}
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
