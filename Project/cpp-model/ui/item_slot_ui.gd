class_name ItemSlotUI extends PanelContainer
## 单个物品槽 UI（纯代码构建，无需场景文件）

const SLOT_SIZE := Vector2(72, 92)

const TAG_COLORS := {
	"weapon":     Color(0.80, 0.25, 0.20),
	"armor":      Color(0.22, 0.45, 0.80),
	"consumable": Color(0.20, 0.65, 0.30),
	"currency":   Color(0.90, 0.75, 0.10),
	"container":  Color(0.60, 0.42, 0.18),
	"equipment":  Color(0.50, 0.30, 0.70),
}
const DEFAULT_COLOR := Color(0.30, 0.30, 0.35)

var _item_id: int = 0
var _type_id: int = 0

var _icon_panel: Panel
var _icon_label: Label
var _name_label: Label
var _count_label: Label

signal slot_hovered(item_id: int)
signal slot_unhovered

func _init() -> void:
	custom_minimum_size = SLOT_SIZE
	_build_ui()

func _build_ui() -> void:
	# 外层样式
	var bg := StyleBoxFlat.new()
	bg.bg_color = Color(0.12, 0.12, 0.15)
	bg.border_color = Color(0.30, 0.30, 0.35)
	bg.set_border_width_all(1)
	bg.set_corner_radius_all(4)
	add_theme_stylebox_override("panel", bg)

	var vbox := VBoxContainer.new()
	vbox.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	vbox.add_theme_constant_override("separation", 2)
	add_child(vbox)

	# 图标面板（正方形）
	_icon_panel = Panel.new()
	_icon_panel.custom_minimum_size = Vector2(62, 54)
	_icon_panel.size_flags_horizontal = Control.SIZE_SHRINK_CENTER
	var icon_bg := StyleBoxFlat.new()
	icon_bg.bg_color = DEFAULT_COLOR
	icon_bg.set_corner_radius_all(4)
	_icon_panel.add_theme_stylebox_override("panel", icon_bg)
	vbox.add_child(_icon_panel)

	_icon_label = Label.new()
	_icon_label.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	_icon_label.vertical_alignment = VERTICAL_ALIGNMENT_CENTER
	_icon_label.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	_icon_label.add_theme_font_size_override("font_size", 22)
	_icon_panel.add_child(_icon_label)

	_name_label = Label.new()
	_name_label.horizontal_alignment = HORIZONTAL_ALIGNMENT_CENTER
	_name_label.add_theme_font_size_override("font_size", 11)
	_name_label.add_theme_color_override("font_color", Color(0.9, 0.9, 0.9))
	_name_label.clip_text = true
	vbox.add_child(_name_label)

	_count_label = Label.new()
	_count_label.horizontal_alignment = HORIZONTAL_ALIGNMENT_RIGHT
	_count_label.add_theme_font_size_override("font_size", 10)
	_count_label.add_theme_color_override("font_color", Color(0.7, 0.85, 1.0))
	vbox.add_child(_count_label)

func _ready() -> void:
	mouse_entered.connect(_on_mouse_entered)
	mouse_exited.connect(_on_mouse_exited)

func setup(p_item_id: int) -> void:
	_item_id = p_item_id
	if p_item_id <= 0:
		_show_empty()
		return

	var data: Dictionary = ItemManagerSingleton.get_item_data(p_item_id)
	_type_id = data.get("type_id", 0)
	var item_name: String = data.get("name", "?")
	var stack: int = data.get("stack_count", 1)
	var is_container: bool = data.get("is_container", false)

	# 图标背景颜色
	var color := _get_color(_type_id)
	var icon_style := StyleBoxFlat.new()
	icon_style.bg_color = color
	icon_style.set_corner_radius_all(4)
	_icon_panel.add_theme_stylebox_override("panel", icon_style)
	_icon_label.text = _get_icon(_type_id, is_container)

	_name_label.text = item_name if item_name.length() <= 5 else item_name.left(4) + "…"

	if stack > 1:
		_count_label.text = "x%d" % stack
		_count_label.visible = true
	else:
		_count_label.visible = false

func _show_empty() -> void:
	var s := StyleBoxFlat.new()
	s.bg_color = Color(0.10, 0.10, 0.12)
	s.border_color = Color(0.20, 0.20, 0.23)
	s.set_border_width_all(1)
	s.set_corner_radius_all(4)
	_icon_panel.add_theme_stylebox_override("panel", s)
	_icon_label.text = ""
	_name_label.text = ""
	_count_label.visible = false

func _get_color(p_type_id: int) -> Color:
	var tm: ItemTemplateManager = ItemManagerSingleton.template_manager
	if not tm.has_template(p_type_id):
		return DEFAULT_COLOR
	var tags: Array = tm.get_template_data(p_type_id).get("tags", [])
	for tag in tags:
		if TAG_COLORS.has(tag):
			return TAG_COLORS[tag]
	return DEFAULT_COLOR

func _get_icon(p_type_id: int, is_container: bool) -> String:
	if is_container:
		return "🎒" if p_type_id == 101 else "📦"
	var tm: ItemTemplateManager = ItemManagerSingleton.template_manager
	if not tm.has_template(p_type_id):
		return "?"
	var tags: Array = tm.get_template_data(p_type_id).get("tags", [])
	if "weapon" in tags:   return "⚔"
	if "armor" in tags:    return "🛡"
	if "potion" in tags:   return "🧪"
	if "food" in tags:     return "🍞"
	if "currency" in tags: return "🪙"
	return "◆"

func _on_mouse_entered() -> void:
	if _item_id > 0:
		slot_hovered.emit(_item_id)

func _on_mouse_exited() -> void:
	slot_unhovered.emit()

static func make(p_item_id: int) -> ItemSlotUI:
	var s := ItemSlotUI.new()
	s.setup(p_item_id)
	return s
