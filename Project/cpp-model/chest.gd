extends Node3D
class_name Chest
## 宝箱类 - 展示场景物品如何使用 ItemManager

@export var item_id: int = 0  # 宝箱对应的物品 ID
@export var chest_size: int = 10  # 宝箱容量

@onready var mesh_instance: MeshInstance3D = $MeshInstance3D
@onready var area: Area3D = $Area3D

var is_open: bool = false

func _ready() -> void:
	# 等待 ItemManager 初始化
	await get_tree().process_frame
	
	# 如果没有 ID，创建新的宝箱物品
	if item_id == 0:
		item_id = ItemManagerSingleton.create_item(100)
		ItemManagerSingleton.item_manager.set_item_name(item_id, "宝箱_%s" % name)
		ItemManagerSingleton.item_manager.set_as_container(item_id, chest_size)
		print("[Chest] 创建新宝箱: ID=%d, 容量=%d" % [item_id, chest_size])
		
		# 初始化战利品
		_initialize_loot()
	else:
		# 从存档加载，验证物品是否存在
		if not ItemManagerSingleton.is_valid_item(item_id):
			push_error("[Chest] 无效的物品 ID: %d" % item_id)
			return
		
		print("[Chest] 加载已存在的宝箱: ID=%d" % item_id)
	
	# 连接信号
	if area:
		area.body_entered.connect(_on_body_entered)

func _initialize_loot() -> void:
	"""初始化宝箱战利品"""
	# 添加一些随机物品
	var loot_types = [
		{"type": 1, "name": "铁剑", "count": 1},
		{"type": 2, "name": "金币", "count": 50},
		{"type": 3, "name": "生命药水", "count": 3},
	]
	
	for loot in loot_types:
		var loot_id = ItemManagerSingleton.create_item(loot.type)
		ItemManagerSingleton.item_manager.set_item_name(loot_id, loot.name)
		ItemManagerSingleton.item_manager.set_stack_count(loot_id, loot.count)
		
		var success = ItemManagerSingleton.add_to_container(loot_id, item_id)
		if success:
			print("  添加战利品: %s x%d" % [loot.name, loot.count])

func open() -> void:
	"""打开宝箱"""
	if is_open:
		return
	
	is_open = true
	print("\n[Chest] 打开宝箱 ID=%d" % item_id)
	
	# 显示宝箱内容
	show_contents()
	
	# 这里可以播放动画、音效等
	if mesh_instance:
		var tween = create_tween()
		tween.tween_property(mesh_instance, "position:y", 0.5, 0.3)

func close() -> void:
	"""关闭宝箱"""
	if not is_open:
		return
	
	is_open = false
	print("[Chest] 关闭宝箱 ID=%d" % item_id)
	
	if mesh_instance:
		var tween = create_tween()
		tween.tween_property(mesh_instance, "position:y", 0.0, 0.3)

func show_contents() -> void:
	"""显示宝箱内容"""
	var items = ItemManagerSingleton.get_container_items(item_id)
	print("宝箱内容:")
	
	var item_count = 0
	for i in range(items.size()):
		var item = items[i]
		if item > 0:
			var name = ItemManagerSingleton.item_manager.get_item_name(item)
			var count = ItemManagerSingleton.item_manager.get_stack_count(item)
			print("  槽位 %d: [%d] %s x%d" % [i, item, name, count])
			item_count += 1
	
	if item_count == 0:
		print("  (空宝箱)")

func take_item(slot: int) -> int:
	"""从宝箱取出物品"""
	var item = ItemManagerSingleton.item_manager.get_item_at_slot(item_id, slot)
	if item > 0:
		ItemManagerSingleton.item_manager.remove_from_container(item)
		print("[Chest] 取出物品: [%d] %s" % [item, ItemManagerSingleton.item_manager.get_item_name(item)])
		return item
	return 0

func add_item(new_item_id: int) -> bool:
	"""向宝箱添加物品"""
	var success = ItemManagerSingleton.add_to_container(new_item_id, item_id)
	if success:
		print("[Chest] 添加物品: [%d] %s" % [new_item_id, ItemManagerSingleton.item_manager.get_item_name(new_item_id)])
	return success

func _on_body_entered(body: Node3D) -> void:
	"""玩家靠近时自动打开"""
	if body.name == "Player" or body is CharacterBody3D:
		open()

func _exit_tree() -> void:
	"""场景节点被删除时，决定是否删除物品数据"""
	# 注意：这里不删除物品数据，因为可能只是场景切换
	# 如果要删除（如宝箱被摧毁），手动调用 destroy()
	pass

func destroy() -> void:
	"""摧毁宝箱（同时删除物品数据）"""
	if ItemManagerSingleton.is_valid_item(item_id):
		ItemManagerSingleton.destroy_item(item_id)
		print("[Chest] 宝箱被摧毁: ID=%d" % item_id)
	queue_free()
