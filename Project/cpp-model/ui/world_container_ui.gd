extends CanvasLayer
## 世界物品容器 UI
## 点击地图上的物品后打开，显示该容器内物品
## 支持拖拽物品到背包（跨容器拖拽）

const SlotUI = preload("res://ui/item_slot_ui.gd")

const MAX_SLOTS := 20
const GRID_PADDING := 16.0

# ── 节点引用 ────────────────────────────────────────────────────
var _panel: Panel
var _title_bar: Control
var _title_label: Label
var _grid: GridContainer
var _scroll: ScrollContainer
var _close_btn: Button
var _slots: Array = []

# ── 拖拽状态 ────────────────────────────────────────────────────
var _drag_moving := false
var _drag_offset := Vector2.ZERO

# ── 物品拖拽状态 ────────────────────────────────────────────────
var _item_dragging := false
var _drag_source_slot = null
var _drag_preview: PanelContainer = null

# ── 其他状态 ────────────────────────────────────────────────────
var _container_id: int = -1

## 当物品被取走时发出（用于通知背包刷新等）
signal item_taken(item_id: int, from_container_id: int)

func _ready() -> void:
	layer = 10
	_build_ui()
	visible = false

func _build_ui() -> void:
	_panel = Panel.new()
	var ps := StyleBoxFlat.new()
	ps.bg_color = Color(0.12, 0.10, 0.08, 0.96)
	ps.border_color = Color(0.65, 0.45, 0.15)
	ps.set_border_width_all(2)
	ps.set_corner_radius_all(8)
	_panel.add_theme_stylebox_override("panel", ps)
	_panel.set_anchors_and_offsets_preset(Control.PRESET_TOP_LEFT)
	_panel.position = Vector2(50, 100)
	_panel.size = Vector2(360, 400)
	_panel.clip_contents = true
	add_child(_panel)

	var vbox := VBoxContainer.new()
	vbox.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	vbox.add_theme_constant_override("separation", 0)
	_panel.add_child(vbox)

	# ── 标题栏 ──
	_title_bar = Control.new()
	_title_bar.custom_minimum_size = Vector2(0, 38)
	_title_bar.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	_title_bar.mouse_default_cursor_shape = Control.CURSOR_MOVE
	vbox.add_child(_title_bar)

	var title_bg := ColorRect.new()
	title_bg.color = Color(0.10, 0.08, 0.06, 1.0)
	title_bg.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	title_bg.mouse_filter = Control.MOUSE_FILTER_IGNORE
	_title_bar.add_child(title_bg)

	_title_bar.gui_input.connect(_on_titlebar_input)

	_title_label = Label.new()
	_title_label.text = "  Container"
	_title_label.add_theme_font_size_override("font_size", 16)
	_title_label.add_theme_color_override("font_color", Color(1.0, 0.85, 0.4))
	_title_label.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	_title_label.offset_left = 12
	_title_label.offset_right = -40
	_title_label.vertical_alignment = VERTICAL_ALIGNMENT_CENTER
	_title_bar.add_child(_title_label)

	_close_btn = Button.new()
	_close_btn.text = "X"
	_close_btn.flat = true
	_close_btn.add_theme_font_size_override("font_size", 15)
	_close_btn.add_theme_color_override("font_color", Color(0.8, 0.3, 0.3))
	_close_btn.set_anchors_and_offsets_preset(Control.PRESET_CENTER_RIGHT)
	_close_btn.offset_right = -6
	_close_btn.offset_left = -34
	_close_btn.offset_top = -16
	_close_btn.offset_bottom = 16
	_close_btn.pressed.connect(func(): visible = false)
	_title_bar.add_child(_close_btn)

	var sep := HSeparator.new()
	vbox.add_child(sep)

	# ── 网格区域 ──
	_scroll = ScrollContainer.new()
	_scroll.size_flags_vertical = Control.SIZE_EXPAND_FILL
	_scroll.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	_scroll.horizontal_scroll_mode = ScrollContainer.SCROLL_MODE_DISABLED
	_scroll.vertical_scroll_mode = ScrollContainer.SCROLL_MODE_AUTO
	vbox.add_child(_scroll)

	_grid = GridContainer.new()
	_grid.columns = 4
	_grid.size_flags_horizontal = Control.SIZE_EXPAND_FILL
	_grid.size_flags_vertical = Control.SIZE_EXPAND_FILL
	_grid.add_theme_constant_override("h_separation", 4)
	_grid.add_theme_constant_override("v_separation", 4)
	_scroll.add_child(_grid)

	_panel.resized.connect(_on_panel_resized)

# ════════════════════════════════════════════════════════════════
func _on_titlebar_input(event: InputEvent) -> void:
	if event is InputEventMouseButton:
		var mb := event as InputEventMouseButton
		if mb.button_index == MOUSE_BUTTON_LEFT:
			if mb.pressed:
				_drag_moving = true
				_drag_offset = _panel.position - get_viewport().get_mouse_position()
			else:
				_drag_moving = false
	elif event is InputEventMouseMotion and _drag_moving:
		var mp := get_viewport().get_mouse_position()
		var new_pos := mp + _drag_offset
		var vp := get_viewport().get_visible_rect().size
		new_pos.x = clampf(new_pos.x, 0, vp.x - _panel.size.x)
		new_pos.y = clampf(new_pos.y, 0, vp.y - _panel.size.y)
		_panel.position = new_pos

func _on_panel_resized() -> void:
	if not _grid or _slots.is_empty():
		return
	var sep_val := float(_grid.get_theme_constant("h_separation"))
	var slot_w := float(SlotUI.SLOT_SIZE.x)
	var avail := _panel.size.x - GRID_PADDING
	var cols := maxi(1, int(avail + sep_val) / int(slot_w + sep_val))
	if _grid.columns != cols:
		_grid.columns = cols

# ════════════════════════════════════════════════════════════════
func open_container(container_id: int) -> void:
	_container_id = container_id
	refresh()
	visible = true
	# 定位到屏幕左侧
	await get_tree().process_frame
	var vp := get_viewport().get_visible_rect().size
	_panel.position = Vector2(50, (vp.y - _panel.size.y) * 0.5)
	_on_panel_resized()

func close() -> void:
	visible = false

func get_container_id() -> int:
	return _container_id

func refresh() -> void:
	for child in _grid.get_children():
		child.queue_free()
	_slots.clear()

	if _container_id <= 0:
		return

	var item_ids: Array = ItemManagerSingleton.get_container_items(_container_id)
	var data = ItemManagerSingleton.get_item_data(_container_id)
	var max_s: int = data.get("max_slots", MAX_SLOTS)
	var item_name: String = data.get("name", "Container")
	var count := item_ids.filter(func(x): return x > 0).size()
	_title_label.text = "%s  (%d/%d)" % [item_name, count, max_s]

	for i in range(max_s):
		var item_id: int = item_ids[i] if i < item_ids.size() else 0
		var slot = SlotUI.new()
		slot.setup(item_id)
		slot.gui_input.connect(_on_slot_gui_input.bind(slot))
		_grid.add_child(slot)
		_slots.append(slot)

	_on_panel_resized()

# ════════════════════════════════════════════════════════════════
# 物品拖拽（从世界容器拖出）
# ════════════════════════════════════════════════════════════════
func _on_slot_gui_input(event: InputEvent, slot) -> void:
	if event is InputEventMouseButton:
		var mb := event as InputEventMouseButton
		if mb.button_index == MOUSE_BUTTON_LEFT:
			if mb.pressed and slot.get_item_id() > 0 and not _item_dragging:
				_start_item_drag(slot)
			elif not mb.pressed and _item_dragging:
				_end_item_drag()

func _start_item_drag(source) -> void:
	_item_dragging = true
	_drag_source_slot = source

	_drag_preview = PanelContainer.new()
	_drag_preview.custom_minimum_size = SlotUI.SLOT_SIZE
	_drag_preview.modulate = Color(1, 1, 1, 0.65)
	_drag_preview.z_index = 30
	_drag_preview.mouse_filter = Control.MOUSE_FILTER_IGNORE

	var preview_style := StyleBoxFlat.new()
	preview_style.bg_color = source._get_color(source._type_id)
	preview_style.set_corner_radius_all(4)
	preview_style.set_border_width_all(2)
	preview_style.border_color = Color(1.0, 0.88, 0.3)
	_drag_preview.add_theme_stylebox_override("panel", preview_style)

	var lbl := Label.new()
	lbl.text = source._icon_label.text
	lbl.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	lbl.vertical_alignment = VERTICAL_ALIGNMENT_CENTER
	lbl.add_theme_font_size_override("font_size", 22)
	lbl.mouse_filter = Control.MOUSE_FILTER_IGNORE
	_drag_preview.add_child(lbl)

	add_child(_drag_preview)
	_drag_preview.position = get_viewport().get_mouse_position() - SlotUI.SLOT_SIZE * 0.5
	source.modulate = Color(1, 1, 1, 0.3)

func _end_item_drag() -> void:
	if not _item_dragging:
		return
	_item_dragging = false

	var source_id := 0
	if _drag_source_slot:
		source_id = _drag_source_slot.get_item_id()

	# 发出信号让外部系统处理跨容器拖拽
	if source_id > 0:
		item_taken.emit(source_id, _container_id)

	# 清理
	if _drag_source_slot:
		_drag_source_slot.modulate = Color(1, 1, 1, 1)
		_drag_source_slot = null
	if _drag_preview:
		_drag_preview.queue_free()
		_drag_preview = null

func _input(event: InputEvent) -> void:
	if _item_dragging and event is InputEventMouseButton:
		var mb := event as InputEventMouseButton
		if mb.button_index == MOUSE_BUTTON_LEFT and not mb.pressed:
			_end_item_drag()
			get_viewport().set_input_as_handled()

func _process(_delta: float) -> void:
	if _item_dragging and _drag_preview:
		var mp := get_viewport().get_mouse_position()
		_drag_preview.position = mp - SlotUI.SLOT_SIZE * 0.5
