extends CanvasLayer
## 背包/物品展示 UI
## 按 I 键切换显示/隐藏
## 自动读取玩家背包（item_id 由 InventoryManager 管理）

const COLS := 5           # 每行槽位数
const MAX_SLOTS := 20     # 最多显示槽位数

var _panel: PanelContainer
var _grid: GridContainer
var _tooltip: PanelContainer
var _tooltip_label: Label
var _title_label: Label
var _slots: Array[ItemSlotUI] = []

# 当前展示的容器 ID（-1 = 未打开）
var _container_id: int = -1

func _ready() -> void:
	layer = 10
	_build_ui()
	visible = false

func _build_ui() -> void:
	# ── 背景遮罩（半透明） ──────────────────────────────────────
	var bg_dim := ColorRect.new()
	bg_dim.color = Color(0, 0, 0, 0.45)
	bg_dim.set_anchors_preset(Control.PRESET_FULL_RECT)
	bg_dim.mouse_filter = Control.MOUSE_FILTER_IGNORE
	add_child(bg_dim)

	# ── 主面板 ─────────────────────────────────────────────────
	_panel = PanelContainer.new()
	var panel_style := StyleBoxFlat.new()
	panel_style.bg_color = Color(0.10, 0.10, 0.14, 0.96)
	panel_style.border_color = Color(0.55, 0.45, 0.20)
	panel_style.set_border_width_all(2)
	panel_style.set_corner_radius_all(8)
	panel_style.content_margin_left   = 16
	panel_style.content_margin_right  = 16
	panel_style.content_margin_top    = 12
	panel_style.content_margin_bottom = 16
	_panel.add_theme_stylebox_override("panel", panel_style)
	_panel.set_anchors_preset(Control.PRESET_CENTER)
	_panel.position = Vector2(-220, -220)   # 粗略居中偏移，_ready 后用 set_position
	add_child(_panel)

	var vbox := VBoxContainer.new()
	vbox.add_theme_constant_override("separation", 10)
	_panel.add_child(vbox)

	# 标题行
	var title_row := HBoxContainer.new()
	vbox.add_child(title_row)

	_title_label = Label.new()
	_title_label.text = "🎒  背包"
	_title_label.add_theme_font_size_override("font_size", 18)
	_title_label.add_theme_color_override("font_color", Color(1.0, 0.88, 0.45))
	_title_label.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	title_row.add_child(_title_label)

	var close_btn := Button.new()
	close_btn.text = "✕"
	close_btn.flat = true
	close_btn.add_theme_font_size_override("font_size", 16)
	close_btn.add_theme_color_override("font_color", Color(0.8, 0.3, 0.3))
	close_btn.pressed.connect(func(): visible = false)
	title_row.add_child(close_btn)

	# 分割线
	var sep := HSeparator.new()
	vbox.add_child(sep)

	# 物品格子
	_grid = GridContainer.new()
	_grid.columns = COLS
	_grid.add_theme_constant_override("h_separation", 4)
	_grid.add_theme_constant_override("v_separation", 4)
	vbox.add_child(_grid)

	# 下方提示
	var hint := Label.new()
	hint.text = "[ I ] 关闭"
	hint.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	hint.add_theme_font_size_override("font_size", 11)
	hint.add_theme_color_override("font_color", Color(0.5, 0.5, 0.55))
	vbox.add_child(hint)

	# ── Tooltip ─────────────────────────────────────────────────
	_tooltip = PanelContainer.new()
	var tip_style := StyleBoxFlat.new()
	tip_style.bg_color = Color(0.08, 0.08, 0.12, 0.95)
	tip_style.border_color = Color(0.55, 0.45, 0.20)
	tip_style.set_border_width_all(1)
	tip_style.set_corner_radius_all(4)
	tip_style.content_margin_left   = 10
	tip_style.content_margin_right  = 10
	tip_style.content_margin_top    = 6
	tip_style.content_margin_bottom = 6
	_tooltip.add_theme_stylebox_override("panel", tip_style)
	_tooltip.visible = false
	add_child(_tooltip)

	_tooltip_label = Label.new()
	_tooltip_label.add_theme_font_size_override("font_size", 12)
	_tooltip_label.add_theme_color_override("font_color", Color(0.92, 0.92, 0.95))
	_tooltip_label.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	_tooltip_label.custom_minimum_size = Vector2(220, 0)
	_tooltip.add_child(_tooltip_label)

func _input(event: InputEvent) -> void:
	if event.is_action_pressed("ui_inventory"):
		get_viewport().set_input_as_handled()
		if visible:
			visible = false
		else:
			_show_panel()

func _show_panel() -> void:
	if _container_id > 0:
		refresh(_container_id)
	visible = true
	# 居中显示
	await get_tree().process_frame
	var vp := get_viewport().get_visible_rect().size
	_panel.position = (vp * 0.5) - (_panel.size * 0.5)

func open_container(container_id: int) -> void:
	_container_id = container_id
	refresh(container_id)
	_show_panel()

func refresh(container_id: int) -> void:
	# 清空旧槽位
	for child in _grid.get_children():
		child.queue_free()
	_slots.clear()

	var item_ids: Array = ItemManagerSingleton.get_container_items(container_id)
	var data := ItemManagerSingleton.get_item_data(container_id)
	var max_s: int = data.get("max_slots", MAX_SLOTS)
	_title_label.text = "🎒  %s  (%d/%d)" % [data.get("name", "背包"), item_ids.filter(func(x): return x > 0).size(), max_s]

	for i in range(max_s):
		var item_id: int = item_ids[i] if i < item_ids.size() else 0
		var slot := ItemSlotUI.new()
		slot.setup(item_id)
		slot.slot_hovered.connect(_on_slot_hovered.bind(slot))
		slot.slot_unhovered.connect(_on_slot_unhovered)
		_grid.add_child(slot)
		_slots.append(slot)

func _on_slot_hovered(item_id: int, _slot: ItemSlotUI) -> void:
	if item_id <= 0:
		_tooltip.visible = false
		return
	var data := ItemManagerSingleton.get_item_data(item_id)
	var type_id: int = data.get("type_id", 0)
	var name_str: String = data.get("name", "?")
	var stack: int = data.get("stack_count", 1)
	var desc: String = ItemVisualDB.get_description(type_id)
	var weight: float = 0.0
	var value: int = 0
	var tm: ItemTemplateManager = ItemManagerSingleton.template_manager
	if tm.has_template(type_id):
		var td := tm.get_template_data(type_id)
		weight = td.get("weight", 0.0)
		value = td.get("value", 0)

	var lines: PackedStringArray = [
		"【%s】" % name_str,
		"数量: %d" % stack,
	]
	if weight > 0:
		lines.append("重量: %.1f kg" % weight)
	if value > 0:
		lines.append("价值: %d 金" % value)
	if desc.length() > 0:
		lines.append("")
		lines.append(desc)
	_tooltip_label.text = "\n".join(lines)
	_tooltip.visible = true

func _on_slot_unhovered() -> void:
	_tooltip.visible = false

func _process(_delta: float) -> void:
	if _tooltip.visible:
		var mp := get_viewport().get_mouse_position()
		var vp := get_viewport().get_visible_rect().size
		var tp := mp + Vector2(14, 14)
		if tp.x + _tooltip.size.x > vp.x:
			tp.x = mp.x - _tooltip.size.x - 8
		if tp.y + _tooltip.size.y > vp.y:
			tp.y = mp.y - _tooltip.size.y - 8
		_tooltip.position = tp
