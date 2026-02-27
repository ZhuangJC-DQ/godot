extends Node
## ItemManager 全局单例
## 自动加载，管理所有游戏物品

var item_manager: ItemManager
var template_manager: ItemTemplateManager

func _ready() -> void:
	# 创建 ItemTemplateManager 并加载模板
	template_manager = ItemTemplateManager.new()
	var success := template_manager.load_templates_from_json("res://data/item_templates.json")
	if success:
		print("[ItemManagerSingleton] Templates loaded successfully")
	else:
		push_error("[ItemManagerSingleton] Failed to load item templates!")

	# 创建 ItemManager 实例（会自动使用 ItemTemplateManager 单例）
	item_manager = ItemManager.new()
	print("[ItemManagerSingleton] Singleton initialized")

func _exit_tree() -> void:
	if item_manager:
		item_manager.clear_all_items()
		item_manager.free()
	if template_manager:
		template_manager.clear_all_templates()
		template_manager.free()

# === 便捷访问接口 ===

func create_item(type_id: int) -> int:
	return item_manager.create_item(type_id)

func destroy_item(item_id: int) -> void:
	item_manager.destroy_item(item_id)

func is_valid_item(item_id: int) -> bool:
	return item_manager.is_valid_item(item_id)

func get_item_data(item_id: int) -> Dictionary:
	return item_manager.get_item_data(item_id)

func set_item_data(item_id: int, data: Dictionary) -> void:
	item_manager.set_item_data(item_id, data)

# === 容器操作 ===

func add_to_container(item_id: int, container_id: int, slot: int = -1) -> bool:
	return item_manager.add_to_container(item_id, container_id, slot)

func get_container_items(container_id: int) -> Array:
	return item_manager.get_container_items(container_id)

# === 模板查询（便捷接口）===

func get_template_data(type_id: int) -> Dictionary:
	return template_manager.get_template_data(type_id)

func has_template(type_id: int) -> bool:
	return template_manager.has_template(type_id)

func get_type_ids_by_tag(tag: String) -> Array:
	return template_manager.get_type_ids_by_tag(tag)

# === 调试接口 ===

func print_all_items() -> void:
	item_manager.print_all_items()
