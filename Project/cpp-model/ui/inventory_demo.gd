extends Node
## 背包演示初始化 + 世界物品交互桥接
## 创建演示物品并绑定到 InventoryUI
## 按 I 键打开/关闭背包；点击地图物品打开世界容器 UI

@onready var _inventory_ui = $"../InventoryUI"
@onready var _map_visualizer = $"../MapVisualizer"

var _world_container_ui: Node = null   # WorldContainerUI 实例
var _backpack_id: int = -1

func _ready() -> void:
	await get_tree().process_frame

	# --- 填充演示背包 ---
	_backpack_id = _create_demo_backpack()

	# 绑定到 UI：传递容器 ID，初始隐藏
	_inventory_ui.open_container(_backpack_id)
	_inventory_ui.visible = false

	# --- 创建世界容器 UI ---
	var WorldContainerScript = preload("res://ui/world_container_ui.gd")
	_world_container_ui = WorldContainerScript.new()
	get_parent().add_child(_world_container_ui)

	# 连接信号：世界容器拖出物品 → 放入背包
	_world_container_ui.item_taken.connect(_on_world_item_taken)

	# 连接信号：地图物品被点击 → 打开世界容器 UI
	if _map_visualizer:
		_map_visualizer.map_item_clicked.connect(_on_map_item_clicked)

	print("[InventoryDemo] 准备就绪 — 按 I 键打开背包")

func _on_map_item_clicked(container_id: int) -> void:
	print("[InventoryDemo] 打开世界容器: id=%d" % container_id)
	_world_container_ui.open_container(container_id)
	# 同时打开背包方便拖拽
	if not _inventory_ui.visible:
		_inventory_ui.visible = true

func _on_world_item_taken(item_id: int, from_container_id: int) -> void:
	# 检查鼠标是否在背包面板区域内
	var mp := get_viewport().get_mouse_position()
	var backpack_rect = _inventory_ui.get_panel_rect()
	if backpack_rect.has_point(mp):
		# 从世界容器移到背包
		ItemManagerSingleton.remove_from_container(item_id)
		ItemManagerSingleton.add_to_container(item_id, _backpack_id)
		print("[InventoryDemo] 物品 %d 从容器 %d → 背包 %d" % [item_id, from_container_id, _backpack_id])
		_inventory_ui.refresh(_backpack_id)
		_world_container_ui.refresh()
	else:
		# 没有拖到背包上，物品回到原位
		_world_container_ui.refresh()

func _create_demo_backpack() -> int:
	var im := ItemManagerSingleton

	# 背包本体（type_id=101，20槽）
	var backpack := im.create_item(101)

	# 武器
	var sword   := im.create_item(1)     # 铁剑
	var shield  := im.create_item(5)     # 铁盾

	# 消耗品
	var potion  := im.create_item(3)     # 生命药水 x3
	im.item_manager.set_stack_count(potion, 3)
	var bread   := im.create_item(4)     # 面包 x8
	im.item_manager.set_stack_count(bread, 8)

	# 货币
	var coins   := im.create_item(2)     # 金币 x120
	im.item_manager.set_stack_count(coins, 120)

	# 嵌套容器（宝箱放在背包里）
	var chest   := im.create_item(100)   # 宝箱（10槽）
	var potion2 := im.create_item(3)
	im.item_manager.set_stack_count(potion2, 2)
	var bread2  := im.create_item(4)
	im.add_to_container(potion2, chest)
	im.add_to_container(bread2, chest)

	# 放入背包
	im.add_to_container(sword,  backpack)
	im.add_to_container(shield, backpack)
	im.add_to_container(potion, backpack)
	im.add_to_container(bread,  backpack)
	im.add_to_container(coins,  backpack)
	im.add_to_container(chest,  backpack)

	print("[InventoryDemo] 背包 id=%d 已填充 6 种物品" % backpack)
	return backpack
