extends Node
## 背包演示初始化
## 创建演示物品并绑定到 InventoryUI
## 按 I 键打开/关闭背包

@onready var _inventory_ui = $"../InventoryUI"

var _backpack_id: int = -1

func _ready() -> void:
	await get_tree().process_frame

	# --- 填充演示背包 ---
	_backpack_id = _create_demo_backpack()

	# 绑定到 UI：传递容器 ID，初始隐藏
	_inventory_ui.open_container(_backpack_id)
	_inventory_ui.visible = false

	print("[InventoryDemo] 准备就绪 — 按 I 键打开背包")

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
