/**************************************************************************/
/*  game_ui.h                                                             */
/**************************************************************************/

#pragma once

#include "scene/gui/control.h"

class Player;
class WorldObject;
class Item;

// GameUI - 游戏UI根节点
// 负责管理所有游戏UI层级和面板的显示/隐藏
class GameUI : public Control {
	GDCLASS(GameUI, Control);

public:
	// UI层级（从底到顶）
	enum Layer {
		LAYER_HUD,        // HUD层（血条、小地图、快捷栏）
		LAYER_PANEL,      // 面板层（背包、商店、技能）
		LAYER_POPUP,      // 弹窗层（确认框、提示）
		LAYER_TOOLTIP,    // 提示层（物品悬浮提示）
		LAYER_MAX
	};

private:
	// UI层级容器
	Control *layers[LAYER_MAX] = { nullptr };

	// 当前绑定的玩家
	Player *bound_player = nullptr;

	// 面板状态
	bool panel_open = false;

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	GameUI();
	virtual ~GameUI();

	// === 初始化 ===
	void setup();

	// === 玩家绑定 ===
	void bind_player(Player *p_player);
	void unbind_player();
	Player *get_bound_player() const { return bound_player; }

	// === 层级访问 ===
	Control *get_layer(Layer p_layer) const;

	// === 面板管理 ===
	void show_panel(Control *p_panel);
	void hide_panel(Control *p_panel);
	void hide_all_panels();
	bool is_any_panel_open() const { return panel_open; }

	// === 工具提示 ===
	void show_tooltip(const String &p_text, const Vector2 &p_position);
	void show_item_tooltip(const Ref<Item> &p_item, const Vector2 &p_position);
	void hide_tooltip();

	// === 输入处理 ===
	virtual void gui_input(const Ref<InputEvent> &p_event) override;
};

VARIANT_ENUM_CAST(GameUI::Layer);
