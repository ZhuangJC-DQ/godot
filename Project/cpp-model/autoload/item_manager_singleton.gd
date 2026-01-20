extends Node
## ItemManager 全局单例
## 自动加载，管理所有游戏物品

var item_manager: ItemManager

func _ready() -> void:
	# 创建 ItemManager 实例
	item_manager = ItemManager.new()
	print("[ItemManager] Singleton initialized")
	
	# 初始化物品类型数据（可以从配置文件加载）
	_initialize_item_types()

func _exit_tree() -> void:
	if item_manager:
		item_manager.clear_all_items()
		item_manager.free()

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

# === 初始化物品类型数据 ===

func _initialize_item_types() -> void:
	# 这里可以从配置文件或数据库加载物品类型
	# 暂时留空，由具体使用场景初始化
	pass

# === 调试接口 ===

func print_all_items() -> void:
	item_manager.print_all_items()
