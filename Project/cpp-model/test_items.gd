extends Node3D
## 物品系统测试脚本
## 演示 ItemManager 的各种功能

func _ready() -> void:
	print("\n========== 物品系统测试开始 ==========")
	
	# 等待一帧确保 ItemManager 初始化完成
	await get_tree().process_frame
	
	test_basic_item_creation()
	test_container_system()
	test_nested_containers()
	test_serialization()
	
	print("\n========== 物品系统测试结束 ==========\n")

# === 测试基础物品创建 ===
func test_basic_item_creation() -> void:
	print("\n--- 测试 1: 基础物品创建 ---")
	
	# 创建几个物品
	var sword_id = ItemManagerSingleton.create_item(1)
	ItemManagerSingleton.item_manager.set_item_name(sword_id, "铁剑")
	ItemManagerSingleton.item_manager.set_stack_count(sword_id, 1)
	ItemManagerSingleton.item_manager.set_max_stack(sword_id, 1)
	
	var gold_id = ItemManagerSingleton.create_item(2)
	ItemManagerSingleton.item_manager.set_item_name(gold_id, "金币")
	ItemManagerSingleton.item_manager.set_stack_count(gold_id, 100)
	ItemManagerSingleton.item_manager.set_max_stack(gold_id, 999)
	
	var potion_id = ItemManagerSingleton.create_item(3)
	ItemManagerSingleton.item_manager.set_item_name(potion_id, "生命药水")
	ItemManagerSingleton.item_manager.set_stack_count(potion_id, 5)
	ItemManagerSingleton.item_manager.set_max_stack(potion_id, 10)
	
	# 打印物品信息
	print("创建了 %d 个物品" % ItemManagerSingleton.item_manager.get_item_count())
	ItemManagerSingleton.item_manager.print_item(sword_id)
	ItemManagerSingleton.item_manager.print_item(gold_id)
	ItemManagerSingleton.item_manager.print_item(potion_id)

# === 测试容器系统 ===
func test_container_system() -> void:
	print("\n--- 测试 2: 容器系统 ---")
	
	# 创建一个背包
	var backpack_id = ItemManagerSingleton.create_item(100)
	ItemManagerSingleton.item_manager.set_item_name(backpack_id, "冒险者背包")
	ItemManagerSingleton.item_manager.set_as_container(backpack_id, 20)  # 20个槽位
	
	print("创建背包: ID=%d, 槽位=%d" % [backpack_id, ItemManagerSingleton.item_manager.get_max_slots(backpack_id)])
	
	# 创建一些物品并放入背包
	var items = []
	for i in range(5):
		var item_id = ItemManagerSingleton.create_item(10 + i)
		ItemManagerSingleton.item_manager.set_item_name(item_id, "物品_%d" % i)
		ItemManagerSingleton.item_manager.set_stack_count(item_id, i + 1)
		items.append(item_id)
		
		# 添加到背包
		var success = ItemManagerSingleton.add_to_container(item_id, backpack_id)
		print("  添加物品 %d 到背包: %s" % [item_id, "成功" if success else "失败"])
	
	# 查看背包内容
	var container_items = ItemManagerSingleton.get_container_items(backpack_id)
	print("\n背包内容 (前10个槽位):")
	for i in range(min(10, container_items.size())):
		var item_id = container_items[i]
		if item_id > 0:
			var name = ItemManagerSingleton.item_manager.get_item_name(item_id)
			var count = ItemManagerSingleton.item_manager.get_stack_count(item_id)
			print("  槽位 %d: [%d] %s x%d" % [i, item_id, name, count])
		else:
			print("  槽位 %d: (空)" % i)

# === 测试嵌套容器 ===
func test_nested_containers() -> void:
	print("\n--- 测试 3: 嵌套容器（容器套容器）---")
	
	# 创建主背包
	var main_bag_id = ItemManagerSingleton.create_item(200)
	ItemManagerSingleton.item_manager.set_item_name(main_bag_id, "主背包")
	ItemManagerSingleton.item_manager.set_as_container(main_bag_id, 30)
	
	# 创建小袋子作为物品
	var coin_pouch_id = ItemManagerSingleton.create_item(201)
	ItemManagerSingleton.item_manager.set_item_name(coin_pouch_id, "钱袋")
	ItemManagerSingleton.item_manager.set_as_container(coin_pouch_id, 5)
	
	# 在钱袋里放金币
	var gold1_id = ItemManagerSingleton.create_item(2)
	ItemManagerSingleton.item_manager.set_item_name(gold1_id, "金币")
	ItemManagerSingleton.item_manager.set_stack_count(gold1_id, 50)
	ItemManagerSingleton.add_to_container(gold1_id, coin_pouch_id)
	
	# 将钱袋放入主背包
	var success = ItemManagerSingleton.add_to_container(coin_pouch_id, main_bag_id)
	print("钱袋放入主背包: %s" % ("成功" if success else "失败"))
	
	# 查看结构
	print("\n容器结构:")
	print("  主背包 [%d]" % main_bag_id)
	var main_items = ItemManagerSingleton.get_container_items(main_bag_id)
	for i in range(min(3, main_items.size())):
		var item_id = main_items[i]
		if item_id > 0:
			var name = ItemManagerSingleton.item_manager.get_item_name(item_id)
			var is_container = ItemManagerSingleton.item_manager.is_container(item_id)
			print("    槽位 %d: [%d] %s %s" % [i, item_id, name, "(容器)" if is_container else ""])
			
			if is_container:
				var sub_items = ItemManagerSingleton.get_container_items(item_id)
				for j in range(sub_items.size()):
					var sub_item_id = sub_items[j]
					if sub_item_id > 0:
						var sub_name = ItemManagerSingleton.item_manager.get_item_name(sub_item_id)
						var sub_count = ItemManagerSingleton.item_manager.get_stack_count(sub_item_id)
						print("      槽位 %d: [%d] %s x%d" % [j, sub_item_id, sub_name, sub_count])

# === 测试序列化 ===
func test_serialization() -> void:
	print("\n--- 测试 4: 序列化与反序列化 ---")
	
	# 保存当前所有物品
	var save_data = ItemManagerSingleton.item_manager.save_to_dict()
	print("保存了 %d 个物品" % save_data["items"].size())
	print("当前物品总数: %d" % ItemManagerSingleton.item_manager.get_item_count())
	
	# 清空并加载
	print("\n清空所有物品...")
	ItemManagerSingleton.item_manager.clear_all_items()
	print("清空后物品总数: %d" % ItemManagerSingleton.item_manager.get_item_count())
	
	print("\n从存档加载...")
	ItemManagerSingleton.item_manager.load_from_dict(save_data)
	print("加载后物品总数: %d" % ItemManagerSingleton.item_manager.get_item_count())
	
	# 验证数据
	print("\n验证加载的物品:")
	ItemManagerSingleton.print_all_items()
