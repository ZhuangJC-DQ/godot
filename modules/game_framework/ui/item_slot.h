/**************************************************************************/
/*  item_slot.h                                                           */
/**************************************************************************/

#pragma once

#include "scene/gui/control.h"
#include "scene/gui/texture_rect.h"
#include "scene/gui/label.h"

class Item;

// ItemSlot - 物品格子控件
// 用于显示单个物品，支持拖拽、点击、悬浮提示
class ItemSlot : public Control {
	GDCLASS(ItemSlot, Control);

public:
	// 格子状态
	enum SlotState {
		STATE_NORMAL,
		STATE_HOVERED,
		STATE_PRESSED,
		STATE_DISABLED,
		STATE_LOCKED,
	};

private:
	// 绑定的物品
	Ref<Item> item;

	// 槽位索引（在容器中的位置）
	int32_t slot_index = -1;

	// 状态
	SlotState state = STATE_NORMAL;
	bool draggable = true;
	bool droppable = true;

	// 子控件
	TextureRect *icon = nullptr;
	Label *quantity_label = nullptr;
	Control *highlight = nullptr;

	// 样式
	Ref<Texture2D> empty_texture;
	Color rarity_colors[6];  // 对应 ItemRarity

	void _update_display();

protected:
	static void _bind_methods();
	void _notification(int p_what);

	// 拖拽支持
	virtual Variant get_drag_data(const Point2 &p_point) override;
	virtual bool can_drop_data(const Point2 &p_point, const Variant &p_data) const override;
	virtual void drop_data(const Point2 &p_point, const Variant &p_data) override;

public:
	ItemSlot();
	virtual ~ItemSlot();

	// === 物品绑定 ===
	void set_item(const Ref<Item> &p_item);
	Ref<Item> get_item() const { return item; }
	bool has_item() const { return item.is_valid(); }
	void clear_item();

	// === 槽位索引 ===
	void set_slot_index(int32_t p_index) { slot_index = p_index; }
	int32_t get_slot_index() const { return slot_index; }

	// === 状态 ===
	void set_slot_state(SlotState p_state);
	SlotState get_slot_state() const { return state; }

	void set_draggable(bool p_draggable) { draggable = p_draggable; }
	bool is_draggable() const { return draggable; }

	void set_droppable(bool p_droppable) { droppable = p_droppable; }
	bool is_droppable() const { return droppable; }

	// === 样式 ===
	void set_empty_texture(const Ref<Texture2D> &p_texture);
	Ref<Texture2D> get_empty_texture() const { return empty_texture; }

	void set_rarity_color(int p_rarity, const Color &p_color);
	Color get_rarity_color(int p_rarity) const;

	// === 输入 ===
	virtual void gui_input(const Ref<InputEvent> &p_event) override;
};

VARIANT_ENUM_CAST(ItemSlot::SlotState);
