extends Node
## 物品模板系统测试脚本
## 验证 C++ ItemTemplateManager + GDScript ItemVisualDB 的完整流程

func _ready() -> void:
	# 等一帧确保所有 Autoload 初始化完成
	await get_tree().process_frame

	print("\n")
	print("========================================")
	print("=== 物品模板系统测试 ===")
	print("========================================\n")

	test_template_loading()
	test_create_item_from_template()
	test_visual_db()
	test_tag_query()
	test_container_template()
	test_full_workflow()

	print("\n========================================")
	print("=== 所有测试完成 ===")
	print("========================================\n")

func test_template_loading() -> void:
	print("--- 测试 1: 模板加载 ---")
	var tm := ItemManagerSingleton.template_manager

	var count := tm.get_template_count()
	print("  模板数量: %d" % count)
	assert(count > 0, "模板数量应 > 0")

	assert(tm.has_template(1), "应有 type_id=1 的模板")
	assert(tm.has_template(100), "应有 type_id=100 的模板")
	assert(not tm.has_template(9999), "不应有 type_id=9999 的模板")

	var data := tm.get_template_data(1)
	print("  铁剑模板: %s" % str(data))
	assert(data["name"] == "铁剑", "名称应为铁剑")
	assert(data["max_stack"] == 1, "铁剑最大堆叠应为 1")

	var data2 := tm.get_template_data(2)
	print("  金币模板: %s" % str(data2))
	assert(data2["max_stack"] == 999, "金币最大堆叠应为 999")

	print("  [PASS] 模板加载测试通过\n")

func test_create_item_from_template() -> void:
	print("--- 测试 2: 从模板创建物品 ---")

	# 创建铁剑（type_id=1）
	var sword_id := ItemManagerSingleton.create_item(1)
	var sword_data := ItemManagerSingleton.get_item_data(sword_id)
	print("  创建铁剑: %s" % str(sword_data))
	assert(sword_data["name"] == "铁剑", "铁剑名称应自动填充")
	assert(sword_data["max_stack"] == 1, "铁剑 max_stack 应自动填充为 1")

	# 创建金币（type_id=2）
	var coin_id := ItemManagerSingleton.create_item(2)
	var coin_data := ItemManagerSingleton.get_item_data(coin_id)
	print("  创建金币: %s" % str(coin_data))
	assert(coin_data["name"] == "金币", "金币名称应自动填充")
	assert(coin_data["max_stack"] == 999, "金币 max_stack 应自动填充为 999")

	# 创建无模板物品（type_id=9999）
	var unknown_id := ItemManagerSingleton.create_item(9999)
	var unknown_data := ItemManagerSingleton.get_item_data(unknown_id)
	print("  创建无模板物品: %s" % str(unknown_data))
	assert(unknown_data["name"] == "", "无模板物品名称应为空")

	# 清理
	ItemManagerSingleton.destroy_item(sword_id)
	ItemManagerSingleton.destroy_item(coin_id)
	ItemManagerSingleton.destroy_item(unknown_id)

	print("  [PASS] 从模板创建物品测试通过\n")

func test_visual_db() -> void:
	print("--- 测试 3: 表现层数据 (ItemVisualDB) ---")

	assert(ItemVisualDB.has_visual(1), "应有 type_id=1 的表现层数据")
	assert(ItemVisualDB.has_visual(100), "应有 type_id=100 的表现层数据")

	var desc: String = ItemVisualDB.get_description(1)
	print("  铁剑描述: %s" % desc)
	assert(desc.length() > 0, "铁剑描述不应为空")

	var desc2: String = ItemVisualDB.get_description(3)
	print("  生命药水描述: %s" % desc2)
	assert(desc2.length() > 0, "生命药水描述不应为空")

	# 无 visual 的物品
	assert(not ItemVisualDB.has_visual(9999), "不应有 type_id=9999 的表现层数据")

	ItemVisualDB.print_all_visuals()
	print("  [PASS] 表现层数据测试通过\n")

func test_tag_query() -> void:
	print("--- 测试 4: 标签查询 ---")

	var weapons := ItemManagerSingleton.get_type_ids_by_tag("weapon")
	print("  weapon 标签: %s" % str(weapons))
	assert(weapons.size() > 0, "应至少有一个 weapon")

	var consumables := ItemManagerSingleton.get_type_ids_by_tag("consumable")
	print("  consumable 标签: %s" % str(consumables))
	assert(consumables.size() > 0, "应至少有一个 consumable")

	var containers := ItemManagerSingleton.get_type_ids_by_tag("container")
	print("  container 标签: %s" % str(containers))
	assert(containers.size() >= 2, "应至少有两个 container (宝箱+背包)")

	var nonexist := ItemManagerSingleton.get_type_ids_by_tag("nonexistent_tag")
	assert(nonexist.size() == 0, "不存在的标签应返回空数组")

	print("  [PASS] 标签查询测试通过\n")

func test_container_template() -> void:
	print("--- 测试 5: 容器模板 ---")

	# 从模板创建宝箱（type_id=100，is_container=true, max_slots=10）
	var chest_id := ItemManagerSingleton.create_item(100)
	var chest_data := ItemManagerSingleton.get_item_data(chest_id)
	print("  创建宝箱: %s" % str(chest_data))
	assert(chest_data["name"] == "宝箱", "宝箱名称应自动填充")
	assert(chest_data["is_container"] == true, "宝箱应自动设为容器")
	assert(chest_data["max_slots"] == 10, "宝箱 max_slots 应自动填充为 10")

	# 往宝箱里放东西
	var potion_id := ItemManagerSingleton.create_item(3)
	var success := ItemManagerSingleton.add_to_container(potion_id, chest_id)
	assert(success, "生命药水应能放入宝箱")
	print("  放入生命药水: success=%s" % str(success))

	# 清理
	ItemManagerSingleton.destroy_item(potion_id)
	ItemManagerSingleton.destroy_item(chest_id)

	print("  [PASS] 容器模板测试通过\n")

func test_full_workflow() -> void:
	print("--- 测试 6: 完整工作流 ---")

	# 模拟：创建背包 → 创建多个物品 → 放入背包 → 查询描述
	var backpack_id := ItemManagerSingleton.create_item(101)
	print("  创建背包 id=%d" % backpack_id)

	var sword_id := ItemManagerSingleton.create_item(1)
	var bread_id := ItemManagerSingleton.create_item(4)
	ItemManagerSingleton.item_manager.set_stack_count(bread_id, 5)

	ItemManagerSingleton.add_to_container(sword_id, backpack_id)
	ItemManagerSingleton.add_to_container(bread_id, backpack_id)

	# 打印背包内容
	var items := ItemManagerSingleton.get_container_items(backpack_id)
	print("  背包内容 (槽位列表): %s" % str(items))

	# 结合表现层，打印每个物品的信息
	for i in range(items.size()):
		var item_id: int = items[i]
		if item_id > 0:
			var data := ItemManagerSingleton.get_item_data(item_id)
			var type_id: int = data["type_id"]
			var desc: String = ItemVisualDB.get_description(type_id)
			print("  槽位%d: %s x%d — %s" % [i, data["name"], data["stack_count"], desc])

	# 清理
	ItemManagerSingleton.destroy_item(sword_id)
	ItemManagerSingleton.destroy_item(bread_id)
	ItemManagerSingleton.destroy_item(backpack_id)

	print("  [PASS] 完整工作流测试通过\n")
