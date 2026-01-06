/**************************************************************************/
/*  player_hud.h                                                          */
/**************************************************************************/

#pragma once

#include "scene/gui/control.h"
#include "scene/gui/label.h"
#include "scene/gui/progress_bar.h"
#include "scene/gui/texture_rect.h"

class Player;

// PlayerHUD - 玩家状态显示
// 显示血量条、法力条、经验条、金币、等级等信息
class PlayerHUD : public Control {
	GDCLASS(PlayerHUD, Control);

public:
	enum BarStyle {
		BAR_STYLE_HORIZONTAL,
		BAR_STYLE_VERTICAL,
	};

private:
	// 绑定的玩家
	Player *bound_player = nullptr;

	// === NodePath配置（可在编辑器中设置） ===
	NodePath health_container_path = NodePath("%HealthContainer");
	NodePath health_bar_path = NodePath("%HealthBar");
	NodePath health_label_path = NodePath("%HealthLabel");

	NodePath mana_container_path = NodePath("%ManaContainer");
	NodePath mana_bar_path = NodePath("%ManaBar");
	NodePath mana_label_path = NodePath("%ManaLabel");

	NodePath exp_container_path = NodePath("%ExpContainer");
	NodePath exp_bar_path = NodePath("%ExpBar");
	NodePath exp_label_path = NodePath("%ExpLabel");

	NodePath info_container_path = NodePath("%InfoContainer");
	NodePath gold_label_path = NodePath("%GoldLabel");
	NodePath level_label_path = NodePath("%LevelLabel");

	// === UI组件缓存（运行时获取） ===
	Control *health_container = nullptr;
	ProgressBar *health_bar = nullptr;
	Label *health_label = nullptr;

	Control *mana_container = nullptr;
	ProgressBar *mana_bar = nullptr;
	Label *mana_label = nullptr;

	Control *exp_container = nullptr;
	ProgressBar *exp_bar = nullptr;
	Label *exp_label = nullptr;

	Control *info_container = nullptr;
	Label *gold_label = nullptr;
	Label *level_label = nullptr;

	// 配置
	bool show_health = true;
	bool show_mana = true;
	bool show_exp = true;
	bool show_gold = true;
	bool show_level = true;
	bool show_numeric_values = true;

	// 样式
	Color health_color = Color(0.8, 0.2, 0.2, 1.0);
	Color mana_color = Color(0.2, 0.4, 0.9, 1.0);
	Color exp_color = Color(0.3, 0.8, 0.3, 1.0);

	// 内部方法
	void _get_ui_nodes(); // 从场景中获取UI节点引用
	void _update_health();
	void _update_mana();
	void _update_exp();
	void _update_gold();
	void _update_level();
	void _update_all();

	// 信号回调
	void _on_player_health_changed();
	void _on_player_mana_changed();
	void _on_player_level_changed(int32_t new_level);
	void _on_player_gold_changed(int64_t old_amount, int64_t new_amount);
	void _on_player_exp_gained(int64_t amount);

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	PlayerHUD();
	virtual ~PlayerHUD();

	// === 玩家绑定 ===
	void bind_player(Player *p_player);
	void unbind_player();
	Player *get_bound_player() const { return bound_player; }

	// === 刷新 ===
	void refresh();

	// === 显示配置 ===
	void set_show_health(bool p_show);
	bool is_show_health() const { return show_health; }

	void set_show_mana(bool p_show);
	bool is_show_mana() const { return show_mana; }

	void set_show_exp(bool p_show);
	bool is_show_exp() const { return show_exp; }

	void set_show_gold(bool p_show);
	bool is_show_gold() const { return show_gold; }

	void set_show_level(bool p_show);
	bool is_show_level() const { return show_level; }

	void set_show_numeric_values(bool p_show);
	bool is_show_numeric_values() const { return show_numeric_values; }

	// === 样式 ===
	void set_health_color(const Color &p_color);
	Color get_health_color() const { return health_color; }

	void set_mana_color(const Color &p_color);
	Color get_mana_color() const { return mana_color; }

	void set_exp_color(const Color &p_color);
	Color get_exp_color() const { return exp_color; }

	// === NodePath访问器（可选，用于编辑器配置） ===
	void set_health_bar_path(const NodePath &p_path) { health_bar_path = p_path; }
	NodePath get_health_bar_path() const { return health_bar_path; }

	void set_health_label_path(const NodePath &p_path) { health_label_path = p_path; }
	NodePath get_health_label_path() const { return health_label_path; }

	void set_mana_bar_path(const NodePath &p_path) { mana_bar_path = p_path; }
	NodePath get_mana_bar_path() const { return mana_bar_path; }

	void set_mana_label_path(const NodePath &p_path) { mana_label_path = p_path; }
	NodePath get_mana_label_path() const { return mana_label_path; }

	void set_exp_bar_path(const NodePath &p_path) { exp_bar_path = p_path; }
	NodePath get_exp_bar_path() const { return exp_bar_path; }

	void set_exp_label_path(const NodePath &p_path) { exp_label_path = p_path; }
	NodePath get_exp_label_path() const { return exp_label_path; }

	void set_gold_label_path(const NodePath &p_path) { gold_label_path = p_path; }
	NodePath get_gold_label_path() const { return gold_label_path; }

	void set_level_label_path(const NodePath &p_path) { level_label_path = p_path; }
	NodePath get_level_label_path() const { return level_label_path; }
};

VARIANT_ENUM_CAST(PlayerHUD::BarStyle);
