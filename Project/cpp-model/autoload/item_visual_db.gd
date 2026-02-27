extends Node

const ItemVisualTemplateScript = preload("res://data/item_visuals/item_visual_template.gd")

var _visuals: Dictionary = {}
const VISUALS_DIR := "res://data/item_visuals/"

func _ready() -> void:
	_load_all_visuals()
	print("[ItemVisualDB] Initialized with %d visual templates" % _visuals.size())

func _load_all_visuals() -> void:
	var dir := DirAccess.open(VISUALS_DIR)
	if not dir:
		push_error("[ItemVisualDB] Cannot open directory: %s" % VISUALS_DIR)
		return
	dir.list_dir_begin()
	var file_name := dir.get_next()
	while file_name != "":
		if not dir.current_is_dir() and file_name.ends_with(".tres"):
			var path := VISUALS_DIR + file_name
			var res = load(path)
			if res and res.get_script() == ItemVisualTemplateScript:
				_visuals[res.type_id] = res
				print("[ItemVisualDB]   Loaded visual: type_id=%d from %s" % [res.type_id, file_name])
			else:
				push_warning("[ItemVisualDB] Skipping: %s" % path)
		file_name = dir.get_next()
	dir.list_dir_end()

func get_visual(type_id: int) -> Resource:
	if _visuals.has(type_id):
		return _visuals[type_id]
	return null

func get_icon(type_id: int) -> Texture2D:
	var visual = get_visual(type_id)
	if visual:
		return visual.icon
	return null

func get_description(type_id: int) -> String:
	var visual = get_visual(type_id)
	if visual:
		return visual.description
	return ""

func has_visual(type_id: int) -> bool:
	return _visuals.has(type_id)

func print_all_visuals() -> void:
	print("=== ItemVisualDB: %d visuals ===" % _visuals.size())
	for type_id in _visuals:
		var v = _visuals[type_id]
		print("  type_id=%d, desc=%s, has_icon=%s" % [v.type_id, v.description.left(30), str(v.icon != null)])
