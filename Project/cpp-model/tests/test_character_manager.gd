extends SceneTree
## CharacterManager GDScript 测试脚本
## 用法: ./bin/Mu --headless --path ./Project/cpp-model --script res://tests/test_character_manager.gd

var _pass_count: int = 0
var _fail_count: int = 0
var _total_count: int = 0

func _init() -> void:
	print("\n========== CharacterManager GDScript 测试开始 ==========\n")

	test_create_and_destroy()
	test_character_types()
	test_basic_properties()
	test_combat_damage_and_heal()
	test_lethal_damage()
	test_inventory_equipment_ids()
	test_custom_properties()
	test_batch_data()
	test_serialization()
	test_get_characters_by_type()
	test_invalid_operations()

	print("\n========== 测试结果 ==========")
	print("通过: %d / %d" % [_pass_count, _total_count])
	if _fail_count > 0:
		print("失败: %d" % _fail_count)
		print("FAIL")
	else:
		print("ALL TESTS PASSED")
	print("========== CharacterManager GDScript 测试结束 ==========\n")
	quit()


func assert_equal(actual, expected, description: String) -> void:
	_total_count += 1
	if actual == expected:
		_pass_count += 1
	else:
		_fail_count += 1
		print("  FAIL: %s — expected %s, got %s" % [description, str(expected), str(actual)])


func assert_true(value: bool, description: String) -> void:
	assert_equal(value, true, description)


func assert_false(value: bool, description: String) -> void:
	assert_equal(value, false, description)


# === 测试 1: 创建与销毁 ===
func test_create_and_destroy() -> void:
	print("--- 测试 1: 创建与销毁 ---")
	var mgr = CharacterManager.new()

	var id = mgr.create_character(CharacterManager.CHARACTER_TYPE_PLAYER)
	assert_true(id > 0, "create_character returns positive ID")
	assert_true(mgr.is_valid_character(id), "character is valid after creation")
	assert_equal(mgr.get_character_count(), 1, "character count is 1")

	mgr.destroy_character(id)
	assert_false(mgr.is_valid_character(id), "character invalid after destroy")
	assert_equal(mgr.get_character_count(), 0, "character count is 0")

	mgr.free()


# === 测试 2: 角色类型 ===
func test_character_types() -> void:
	print("--- 测试 2: 角色类型 ---")
	var mgr = CharacterManager.new()

	var player_id = mgr.create_character(CharacterManager.CHARACTER_TYPE_PLAYER)
	var npc_id = mgr.create_character(CharacterManager.CHARACTER_TYPE_NPC)
	var enemy_id = mgr.create_character(CharacterManager.CHARACTER_TYPE_ENEMY)

	assert_equal(mgr.get_character_type(player_id), CharacterManager.CHARACTER_TYPE_PLAYER, "player type")
	assert_equal(mgr.get_character_type(npc_id), CharacterManager.CHARACTER_TYPE_NPC, "npc type")
	assert_equal(mgr.get_character_type(enemy_id), CharacterManager.CHARACTER_TYPE_ENEMY, "enemy type")

	mgr.free()


# === 测试 3: 基础属性 ===
func test_basic_properties() -> void:
	print("--- 测试 3: 基础属性 ---")
	var mgr = CharacterManager.new()
	var id = mgr.create_character(CharacterManager.CHARACTER_TYPE_PLAYER)

	mgr.set_character_name(id, "勇者")
	assert_equal(mgr.get_character_name(id), "勇者", "name getter")

	mgr.set_level(id, 10)
	assert_equal(mgr.get_level(id), 10, "level getter")

	mgr.set_max_hp(id, 200)
	assert_equal(mgr.get_max_hp(id), 200, "max_hp getter")

	mgr.set_hp(id, 150)
	assert_equal(mgr.get_hp(id), 150, "hp getter")

	mgr.set_attack(id, 25)
	assert_equal(mgr.get_attack(id), 25, "attack getter")

	mgr.set_defense(id, 15)
	assert_equal(mgr.get_defense(id), 15, "defense getter")

	mgr.set_speed(id, 8.5)
	assert_equal(mgr.get_speed(id), 8.5, "speed getter")

	mgr.free()


# === 测试 4: 战斗系统 —— 伤害与治疗 ===
func test_combat_damage_and_heal() -> void:
	print("--- 测试 4: 战斗系统 —— 伤害与治疗 ---")
	var mgr = CharacterManager.new()
	var id = mgr.create_character(CharacterManager.CHARACTER_TYPE_PLAYER)

	mgr.set_defense(id, 3)
	var actual_damage = mgr.take_damage(id, 10)
	assert_equal(actual_damage, 7, "damage reduced by defense (10-3=7)")
	assert_equal(mgr.get_hp(id), 93, "hp after damage (100-7=93)")

	mgr.heal(id, 5)
	assert_equal(mgr.get_hp(id), 98, "hp after heal (93+5=98)")

	# Damage less than defense
	actual_damage = mgr.take_damage(id, 2)
	assert_equal(actual_damage, 0, "damage less than defense = 0")
	assert_equal(mgr.get_hp(id), 98, "hp unchanged when damage < defense")

	mgr.free()


# === 测试 5: 致命伤害 ===
func test_lethal_damage() -> void:
	print("--- 测试 5: 致命伤害 ---")
	var mgr = CharacterManager.new()
	var id = mgr.create_character(CharacterManager.CHARACTER_TYPE_PLAYER)

	mgr.set_defense(id, 0)
	mgr.take_damage(id, 200)
	assert_equal(mgr.get_hp(id), 0, "hp is 0 after lethal damage")
	assert_false(mgr.is_alive(id), "character is dead")

	# Heal should not work on dead character
	mgr.heal(id, 50)
	assert_equal(mgr.get_hp(id), 0, "cannot heal dead character")

	mgr.free()


# === 测试 6: 背包和装备 ID ===
func test_inventory_equipment_ids() -> void:
	print("--- 测试 6: 背包和装备 ID ---")
	var mgr = CharacterManager.new()
	var id = mgr.create_character(CharacterManager.CHARACTER_TYPE_PLAYER)

	assert_equal(mgr.get_inventory_id(id), 0, "default inventory_id is 0")
	assert_equal(mgr.get_equipment_id(id), 0, "default equipment_id is 0")

	mgr.set_inventory_id(id, 42)
	mgr.set_equipment_id(id, 43)

	assert_equal(mgr.get_inventory_id(id), 42, "inventory_id after set")
	assert_equal(mgr.get_equipment_id(id), 43, "equipment_id after set")

	mgr.free()


# === 测试 7: 自定义属性 ===
func test_custom_properties() -> void:
	print("--- 测试 7: 自定义属性 ---")
	var mgr = CharacterManager.new()
	var id = mgr.create_character(CharacterManager.CHARACTER_TYPE_NPC)

	assert_false(mgr.has_custom_property(id, "quest_giver"), "no custom prop initially")

	mgr.set_custom_property(id, "quest_giver", true)
	assert_true(mgr.has_custom_property(id, "quest_giver"), "has custom prop after set")
	assert_equal(mgr.get_custom_property(id, "quest_giver"), true, "custom prop value")

	mgr.set_custom_property(id, "dialogue_id", 42)
	assert_equal(mgr.get_custom_property(id, "dialogue_id"), 42, "int custom prop")

	mgr.remove_custom_property(id, "quest_giver")
	assert_false(mgr.has_custom_property(id, "quest_giver"), "custom prop removed")

	mgr.free()


# === 测试 8: 批量数据操作 ===
func test_batch_data() -> void:
	print("--- 测试 8: 批量数据操作 ---")
	var mgr = CharacterManager.new()
	var id = mgr.create_character(CharacterManager.CHARACTER_TYPE_PLAYER)
	mgr.set_character_name(id, "测试英雄")
	mgr.set_level(id, 10)
	mgr.set_attack(id, 30)

	var data = mgr.get_character_data(id)
	assert_equal(data["name"], "测试英雄", "batch get name")
	assert_equal(data["level"], 10, "batch get level")
	assert_equal(data["attack"], 30, "batch get attack")
	assert_equal(data["type"], CharacterManager.CHARACTER_TYPE_PLAYER, "batch get type")

	# Set via batch
	var new_data = {"name": "数据英雄", "level": 20, "max_hp": 500, "hp": 400}
	mgr.set_character_data(id, new_data)
	assert_equal(mgr.get_character_name(id), "数据英雄", "batch set name")
	assert_equal(mgr.get_level(id), 20, "batch set level")
	assert_equal(mgr.get_max_hp(id), 500, "batch set max_hp")
	assert_equal(mgr.get_hp(id), 400, "batch set hp")

	mgr.free()


# === 测试 9: 序列化与反序列化 ===
func test_serialization() -> void:
	print("--- 测试 9: 序列化与反序列化 ---")
	var mgr = CharacterManager.new()

	var p_id = mgr.create_character(CharacterManager.CHARACTER_TYPE_PLAYER)
	mgr.set_character_name(p_id, "存档英雄")
	mgr.set_level(p_id, 15)
	mgr.set_attack(p_id, 40)

	var n_id = mgr.create_character(CharacterManager.CHARACTER_TYPE_NPC)
	mgr.set_character_name(n_id, "商人")

	var save_data = mgr.save_to_dict()

	# Clear and reload
	mgr.load_from_dict(save_data)

	assert_equal(mgr.get_character_count(), 2, "character count after load")
	assert_true(mgr.is_valid_character(p_id), "player valid after load")
	assert_equal(mgr.get_character_name(p_id), "存档英雄", "player name after load")
	assert_equal(mgr.get_level(p_id), 15, "player level after load")
	assert_equal(mgr.get_attack(p_id), 40, "player attack after load")
	assert_equal(mgr.get_character_type(p_id), CharacterManager.CHARACTER_TYPE_PLAYER, "player type after load")

	assert_true(mgr.is_valid_character(n_id), "npc valid after load")
	assert_equal(mgr.get_character_name(n_id), "商人", "npc name after load")
	assert_equal(mgr.get_character_type(n_id), CharacterManager.CHARACTER_TYPE_NPC, "npc type after load")

	mgr.free()


# === 测试 10: 按类型获取角色 ===
func test_get_characters_by_type() -> void:
	print("--- 测试 10: 按类型获取角色 ---")
	var mgr = CharacterManager.new()

	mgr.create_character(CharacterManager.CHARACTER_TYPE_PLAYER)
	mgr.create_character(CharacterManager.CHARACTER_TYPE_NPC)
	mgr.create_character(CharacterManager.CHARACTER_TYPE_NPC)
	mgr.create_character(CharacterManager.CHARACTER_TYPE_ENEMY)

	var players = mgr.get_characters_by_type(CharacterManager.CHARACTER_TYPE_PLAYER)
	assert_equal(players.size(), 1, "1 player")

	var npcs = mgr.get_characters_by_type(CharacterManager.CHARACTER_TYPE_NPC)
	assert_equal(npcs.size(), 2, "2 npcs")

	var enemies = mgr.get_characters_by_type(CharacterManager.CHARACTER_TYPE_ENEMY)
	assert_equal(enemies.size(), 1, "1 enemy")

	var all_ids = mgr.get_all_character_ids()
	assert_equal(all_ids.size(), 4, "4 total characters")

	mgr.free()


# === 测试 11: 无效操作 ===
func test_invalid_operations() -> void:
	print("--- 测试 11: 无效操作 ---")
	var mgr = CharacterManager.new()

	assert_false(mgr.is_valid_character(999), "invalid id returns false")
	assert_equal(mgr.get_character_count(), 0, "empty manager count is 0")

	mgr.free()
