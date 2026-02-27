extends CanvasLayer
## 背包/物品展示 UI
## 按 I 键切换显示/隐藏；标题栏拖拽移动；右下角拖拽缩放

const COLS := 5
const MAX_SLOTS := 20
const MIN_SIZE := Vector2(300, 200)   # 最小窗口尺寸
const GRID_PADDING := 16.0            # 网格左右各 8px 内边距之和

# ── 节点引用 ────────────────────────────────────────────────────
var _panel: Panel                    # 根容器（改用 Panel 以便自由控制 size）
var _title_bar: Control              # 标题栏拖拽区
var _title_label: Label
var _grid: GridContainer
var _scroll: ScrollContainer         # 网格可滚动
var _resize_handle: Control          # 右下角缩放手柄
var _tooltip: PanelContainer
var _tooltip_label: Label
var _slots: Array[ItemSlotUI] = []

# ── 拖拽状态 ────────────────────────────────────────────────────
var _drag_moving  := false
var _drag_offset  := Vector2.ZERO
var _drag_resizing := false
var _resize_start_mouse := Vector2.ZERO
var _resize_start_size  := Vector2.ZERO
var _resize_start_pos   := Vector2.ZERO

# ── 其他状态 ────────────────────────────────────────────────────
var _container_id: int = -1

# ════════════════════════════════════════════════════════════════
func _ready() -> void:
	layer = 10
	_build_ui()
	visible = false

# ════════════════════════════════════════════════════════════════
func _build_ui() -> void:

	# ── 主面板（Panel，不用 PanelContainer 以便手动控制 size）──
	_panel = Panel.new()
	var ps := StyleBoxFlat.new()
	ps.bg_color         = Color(0.10, 0.10, 0.14, 0.96)
	ps.border_color     = Color(0.55, 0.45, 0.20)
	ps.set_border_width_all(2)
	ps.set_corner_radius_all(8)
	_panel.add_theme_stylebox_override("panel", ps)
	_panel.set_anchors_and_offsets_preset(Control.PRESET_TOP_LEFT)
	_panel.position = Vector2(100, 100)
	_panel.size      = Vector2(420, 460)
	_panel.clip_contents = true
	add_child(_panel)

	# ── 垂直布局 ────────────────────────────────────────────────
	var vbox := VBoxContainer.new()
	vbox.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	vbox.add_theme_constant_override("separation", 0)
	_panel.add_child(vbox)

	# ── 标题栏（可拖拽区域）────────────────────────────────────
	_title_bar = Control.new()
	_title_bar.custom_minimum_size = Vector2(0, 38)
	_title_bar.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	_title_bar.mouse_default_cursor_shape = Control.CURSOR_MOVE
	var title_bg := StyleBoxFlat.new()
	title_bg.bg_color = Color(0.08, 0.08, 0.11, 1.0)
	title_bg.corner_radius_top_left  = 8
	title_bg.corner_radius_top_right = 8
	title_bg.border_color = Color(0.55, 0.45, 0.20)
	title_bg.border_width_bottom = 1
	_title_bar.add_theme_stylebox_override("panel", title_bg)   # 仅作背景色用
	vbox.add_child(_title_bar)
	# 把背景色套在一个 PanelContainer 外壳上才能显示，改用 ColorRect
	var title_bg_rect := ColorRect.new()
	title_bg_rect.color = Color(0.08, 0.08, 0.11, 1.0)
	title_bg_rect.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	title_bg_rect.mouse_filter = Control.MOUSE_FILTER_IGNORE
	_title_bar.add_child(title_bg_rect)

	# 标题拖拽事件
	_title_bar.gui_input.connect(_on_titlebar_input)

	# 标题文字
	_title_label = Label.new()
	_title_label.text = "🎒  背包"
	_title_label.add_theme_font_size_override("font_size", 16)
	_title_label.add_theme_color_override("font_color", Color(1.0, 0.88, 0.45))
	_title_label.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	_title_label.offset_left = 12
	_title_label.offset_right = -40
	_title_label.vertical_alignment = VERTICAL_ALIGNMENT_CENTER
	_title_bar.add_child(_title_label)

	# 关闭按钮
	var close_btn := Button.new()
	close_btn.text = "✕"
	close_btn.flat = true
	close_btn.add_theme_font_size_override("font_size", 15)
	close_btn.add_theme_color_override("font_color", Color(0.8, 0.3, 0.3))
	close_btn.set_anchors_and_offsets_preset(Control.PRESET_CENTER_RIGHT)
	close_btn.offset_right  = -6
	close_btn.offset_left   = -34
	close_btn.offset_top    = -16
	close_btn.offset_bottom = 16
	close_btn.pressed.connect(func(): visible = false)
	_title_bar.add_child(close_btn)

	# ── 分割线 ─────────────────────────────────────────────────
	var sep := HSeparator.new()
	sep.add_theme_color_override("color", Color(0.55, 0.45, 0.20, 0.6))
	vbox.add_child(sep)

	# ── 可滚动的网格区域 ────────────────────────────────────────
	_scroll = ScrollContainer.new()
	_scroll.size_flags_vertical    = Control.SIZE_EXPAND_FILL
	_scroll.size_flags_horizontal  = Control.SIZE_EXPAND_FILL
	_scroll.horizontal_scroll_mode = ScrollContainer.SCROLL_MODE_DISABLED
	_scroll.vertical_scroll_mode   = ScrollContainer.SCROLL_MODE_AUTO
	vbox.add_child(_scroll)

	_grid = GridContainer.new()
	_grid.columns = COLS
	_grid.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	_grid.size_flags_vertical   = Control.SIZE_EXPAND_FILL
	# 用 theme 常量做内边距（替代 MarginContainer）
	_grid.add_theme_constant_override("h_separation", 4)
	_grid.add_theme_constant_override("v_separation", 4)
	_scroll.add_child(_grid)

	_panel.resized.connect(_on_panel_resized)
	# ScrollContainer 不需要再监听，通过 panel.resized 驱动

	# ── 底部提示栏 ─────────────────────────────────────────────
	var bottom_bar := Control.new()
	bottom_bar.custom_minimum_size = Vector2(0, 22)
	vbox.add_child(bottom_bar)

	var hint := Label.new()
	hint.text = "[ I ] 关闭"
	hint.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	hint.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	hint.add_theme_font_size_override("font_size", 11)
	hint.add_theme_color_override("font_color", Color(0.45, 0.45, 0.50))
	bottom_bar.add_child(hint)

	# ── 右下角缩放手柄（挂在 CanvasLayer 根，避免被 Panel 内容遮挡）──
	_resize_handle = Control.new()
	_resize_handle.size = Vector2(20, 20)
	_resize_handle.mouse_default_cursor_shape = Control.CURSOR_FDIAGSIZE
	_resize_handle.mouse_filter = Control.MOUSE_FILTER_STOP
	_resize_handle.z_index = 15
	add_child(_resize_handle)   # 挂在 CanvasLayer，不被 Panel 内部节点遮挡
	_resize_handle.gui_input.connect(_on_resize_input)

	# 手柄图案（三条斜线）
	var handle_draw := _ResizeHandleDrawer.new()
	handle_draw.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	handle_draw.mouse_filter = Control.MOUSE_FILTER_IGNORE
	_resize_handle.add_child(handle_draw)

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
	_tooltip.z_index  = 20
	add_child(_tooltip)

	_tooltip_label = Label.new()
	_tooltip_label.add_theme_font_size_override("font_size", 12)
	_tooltip_label.add_theme_color_override("font_color", Color(0.92, 0.92, 0.95))
	_tooltip_label.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	_tooltip_label.custom_minimum_size = Vector2(220, 0)
	_tooltip.add_child(_tooltip_label)

# ════════════════════════════════════════════════════════════════
# 拖拽移动
# ════════════════════════════════════════════════════════════════
func _on_titlebar_input(event: InputEvent) -> void:
	if event is InputEventMouseButton:
		var mb := event as InputEventMouseButton
		if mb.button_index == MOUSE_BUTTON_LEFT:
			if mb.pressed:
				_drag_moving = true
				_drag_offset = _panel.position - get_viewport().get_mouse_position()
				_title_bar.mouse_default_cursor_shape = Control.CURSOR_MOVE
			else:
				_drag_moving = false

	elif event is InputEventMouseMotion and _drag_moving:
		var mp := get_viewport().get_mouse_position()
		var new_pos := mp + _drag_offset
		_panel.position = _clamp_panel_pos(new_pos, _panel.size)

# ════════════════════════════════════════════════════════════════
# 缩放
# ════════════════════════════════════════════════════════════════
func _on_resize_input(event: InputEvent) -> void:
	if event is InputEventMouseButton:
		var mb := event as InputEventMouseButton
		if mb.button_index == MOUSE_BUTTON_LEFT:
			if mb.pressed:
				_drag_resizing = true
				_resize_start_mouse = get_viewport().get_mouse_position()
				_resize_start_size  = _panel.size
				_resize_start_pos   = _panel.position
			else:
				_drag_resizing = false

	elif event is InputEventMouseMotion and _drag_resizing:
		var mp    := get_viewport().get_mouse_position()
		var delta := mp - _resize_start_mouse
		var new_size := (_resize_start_size + delta).max(MIN_SIZE)
		# 限制不超出视口右侧/下侧
		var vp := get_viewport().get_visible_rect().size
		new_size.x = minf(new_size.x, vp.x - _panel.position.x - 4)
		new_size.y = minf(new_size.y, vp.y - _panel.position.y - 4)
		_panel.size = new_size
		_update_resize_handle()

func _update_resize_handle() -> void:
	# 手柄挂在 CanvasLayer 根，需要用面板的绝对位置
	_resize_handle.position = _panel.position + _panel.size - Vector2(20, 20)

func _on_panel_resized() -> void:
	if not _grid or _slots.is_empty():
		return
	# 根据可用宽度动态计算列数：宽度能放几个槽位就放几个
	var sep    := float(_grid.get_theme_constant("h_separation"))
	var slot_w := ItemSlotUI.SLOT_SIZE.x
	var avail  := _panel.size.x - GRID_PADDING
	var cols   := maxi(1, int(avail + sep) / int(slot_w + sep))
	if _grid.columns != cols:
		_grid.columns = cols

# 将面板坐标限制在视口内
func _clamp_panel_pos(pos: Vector2, sz: Vector2) -> Vector2:
	var vp := get_viewport().get_visible_rect().size
	pos.x = clampf(pos.x, 0, vp.x - sz.x)
	pos.y = clampf(pos.y, 0, vp.y - sz.y)
	return pos

# ════════════════════════════════════════════════════════════════
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
	await get_tree().process_frame
	# 居中
	var vp := get_viewport().get_visible_rect().size
	_panel.position = (vp * 0.5) - (_panel.size * 0.5)
	_update_resize_handle()
	_on_panel_resized()

func open_container(container_id: int) -> void:
	_container_id = container_id
	refresh(container_id)
	_show_panel()

func refresh(container_id: int) -> void:
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

	# 槽位创建完毕后立即计算列数
	_on_panel_resized()

# ════════════════════════════════════════════════════════════════
# Tooltip
# ════════════════════════════════════════════════════════════════
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
		value  = td.get("value", 0)

	var lines: PackedStringArray = ["【%s】" % name_str, "数量: %d" % stack]
	if weight > 0: lines.append("重量: %.1f kg" % weight)
	if value  > 0: lines.append("价值: %d 金" % value)
	if desc.length() > 0:
		lines.append("")
		lines.append(desc)
	_tooltip_label.text = "\n".join(lines)
	_tooltip.visible = true

func _on_slot_unhovered() -> void:
	_tooltip.visible = false

func _process(_delta: float) -> void:
	# 实时跟随面板（面板移动/缩放时手柄同步）
	if _resize_handle and visible:
		_resize_handle.position = _panel.position + _panel.size - Vector2(20, 20)
	_resize_handle.visible = visible

	if _tooltip.visible:
		var mp := get_viewport().get_mouse_position()
		var vp := get_viewport().get_visible_rect().size
		var tp := mp + Vector2(14, 14)
		if tp.x + _tooltip.size.x > vp.x: tp.x = mp.x - _tooltip.size.x - 8
		if tp.y + _tooltip.size.y > vp.y: tp.y = mp.y - _tooltip.size.y - 8
		_tooltip.position = tp

# ════════════════════════════════════════════════════════════════
# 右下角缩放手柄绘制（内部类）
# ════════════════════════════════════════════════════════════════
class _ResizeHandleDrawer extends Control:
	func _draw() -> void:
		var c := Color(0.55, 0.45, 0.20, 0.85)
		var w := size.x
		var h := size.y
		var lw := 1.5
		# 三条平行斜线
		for i in range(3):
			var off := 4.0 + i * 5.0
			draw_line(Vector2(w - off, h - 2), Vector2(w - 2, h - off), c, lw)
