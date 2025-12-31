/**************************************************************************/
/*  chunk.cpp                                                             */
/**************************************************************************/

#include "chunk.h"

#include "core/string/print_string.h"
#include "core/variant/variant.h"
#include <cmath>

// 预定义物品ID列表（用于随机生成）
static const char *ITEM_IDS[] = {
	"gold_coin",
	"iron_sword",
	"health_potion",
	"wood_plank",
	"stone_block",
	"magic_scroll",
	"leather_armor",
	"bread"
};
static const int ITEM_COUNT = sizeof(ITEM_IDS) / sizeof(ITEM_IDS[0]);

// 预定义怪物ID列表
static const char *MONSTER_IDS[] = {
	"goblin",
	"wolf",
	"skeleton",
	"orc",
	"troll",
	"spider",
	"bandit",
	"slime"
};
static const int MONSTER_ID_COUNT = sizeof(MONSTER_IDS) / sizeof(MONSTER_IDS[0]);

// 预定义NPC ID列表
static const char *NPC_IDS[] = {
	"blacksmith",
	"merchant",
	"innkeeper",
	"guard",
	"villager",
	"healer",
	"trainer"
};
static const int NPC_ID_COUNT = sizeof(NPC_IDS) / sizeof(NPC_IDS[0]);

Chunk::Chunk(const ChunkCoord &p_coord) :
		coord(p_coord), center_x(0), center_y(0) {
	generate();
}

void Chunk::generate() {
	RandomPCG rng(coord.to_seed());

	// 生成随机中心点
	center_x = rng.rand(CHUNK_SIZE);
	center_y = rng.rand(CHUNK_SIZE);

	// 计算最大可能距离（用于归一化）
	float max_dist = std::sqrt((float)(CHUNK_SIZE * CHUNK_SIZE + CHUNK_SIZE * CHUNK_SIZE));

	// 清空列表
	cities.clear();
	monsters.clear();
	npcs.clear();

	// 遍历每个格子生成地形
	for (int y = 0; y < CHUNK_SIZE; y++) {
		for (int x = 0; x < CHUNK_SIZE; x++) {
			int dx = x - center_x;
			int dy = y - center_y;
			float dist = std::sqrt((float)(dx * dx + dy * dy));
			float normalized_dist = dist / max_dist;

			// 随机扰动
			float noise = rng.randf() * 0.15f;
			normalized_dist += noise;

			// 根据距离决定地形类型
			TileType type;
			if (normalized_dist < 0.08f) {
				type = TILE_CITY;
			} else if (normalized_dist < 0.15f) {
				type = TILE_TOWN;
			} else if (normalized_dist < 0.25f) {
				type = TILE_VILLAGE;
			} else if (normalized_dist < 0.45f) {
				type = TILE_GRASSLAND;
			} else if (normalized_dist < 0.70f) {
				type = TILE_FOREST;
			} else {
				type = TILE_MOUNTAIN;
			}

			tiles[y][x] = type;
		}
	}

	// 生成世界物体（使用同一个 rng 保证可重复性）
	generate_world_objects(rng);
	// 生成怪物
	generate_monsters(rng);
	// 生成 NPC
	generate_npcs(rng);
}

void Chunk::generate_world_objects(RandomPCG &rng) {
	// 收集所有 CITY 格子的位置（采样一些代表性的城市点）
	Vector<Vector2i> city_positions;

	// 为了避免太多城市点，我们按步长采样
	const int sample_step = 8;
	for (int y = 0; y < CHUNK_SIZE; y += sample_step) {
		for (int x = 0; x < CHUNK_SIZE; x += sample_step) {
			if (tiles[y][x] == TILE_CITY) {
				city_positions.push_back(Vector2i(x, y));
			}
		}
	}

	// 限制最大城市数量
	const int max_cities = 5;
	int num_cities = MIN((int)city_positions.size(), max_cities);

	// 随机选择一些城市位置生成 WorldObject
	for (int i = 0; i < num_cities && city_positions.size() > 0; i++) {
		int idx = rng.rand(city_positions.size());
		Vector2i pos = city_positions[idx];
		city_positions.remove_at(idx);

		// 创建城市 WorldObject
		Ref<WorldObject> city_obj;
		city_obj.instantiate();

		StringName city_id = vformat("city_%d_%d_%d_%d", coord.x, coord.y, pos.x, pos.y);
		city_obj->setup(city_id, WorldObject::TYPE_CONTAINER, pos);

		// 初始化容器（3-6个槽位）
		int container_size = 3 + rng.rand(4);
		city_obj->init_container(container_size);

		// 随机添加1-3个物品
		int item_count = 1 + rng.rand(3);
		for (int j = 0; j < item_count; j++) {
			Ref<Item> item;
			item.instantiate();

			// 随机选择物品类型
			int item_idx = rng.rand(ITEM_COUNT);
			item->set_item_id(StringName(ITEM_IDS[item_idx]));
			item->set_display_name(StringName(ITEM_IDS[item_idx]));

			// 随机数量（1-10）
			int quantity = 1 + rng.rand(10);
			item->set_max_stack_size(99);
			item->set_quantity(quantity);

			// 随机稀有度
			item->set_rarity(static_cast<ItemRarity>(rng.rand(5)));

			// 随机类别
			item->set_category(static_cast<ItemCategory>(rng.rand(6)));

			// 添加到容器
			city_obj->container_add_item(item);
		}

		// 保存城市数据
		CityData city_data;
		city_data.position = pos;
		city_data.world_object = city_obj;
		cities.push_back(city_data);
	}
}

String Chunk::to_string(int preview_size) const {
	const char tile_chars[] = {
		'@', // CITY
		'#', // TOWN
		'o', // VILLAGE
		'.', // GRASSLAND
		'T', // FOREST
		'^', // MOUNTAIN
	};

	String result;
	result += vformat("=== Chunk (%d, %d) | Center: (%d, %d) ===\n",
			coord.x, coord.y, center_x, center_y);
	result += vformat("    Cities: %d | Monsters: %d | NPCs: %d\n",
			cities.size(), monsters.size(), npcs.size());
	result += "Legend: @ City  # Town  o Village  . Grass  T Forest  ^ Mountain\n";
	result += String("-").repeat(preview_size + 2) + "\n";

	int step = CHUNK_SIZE / preview_size;
	for (int y = 0; y < preview_size; y++) {
		result += "|";
		for (int x = 0; x < preview_size; x++) {
			int sample_x = x * step;
			int sample_y = y * step;
			TileType type = tiles[sample_y][sample_x];
			result += tile_chars[type];
		}
		result += "|\n";
	}

	result += String("-").repeat(preview_size + 2) + "\n";

	// 列出所有城市
	if (cities.size() > 0) {
		result += "Cities:\n";
		for (int i = 0; i < cities.size(); i++) {
			const CityData &city = cities[i];
			result += vformat("  [%d] Pos: (%d, %d), Items: %d\n",
					i, city.position.x, city.position.y,
					city.world_object.is_valid() ? city.world_object->get_container_used_slots() : 0);
		}
	}

	// 列出所有怪物
	if (monsters.size() > 0) {
		static const char *RANK_NAMES[] = { "Normal", "Elite", "Champion", "Boss", "WorldBoss" };
		result += "Monsters:\n";
		for (int i = 0; i < monsters.size(); i++) {
			const MonsterSpawnData &data = monsters[i];
			if (data.monster.is_valid()) {
				result += vformat("  [%d] %s (%s) Pos: (%d, %d), Items: %d\n",
						i, String(data.monster->get_monster_id()),
						RANK_NAMES[data.monster->get_rank()],
						data.position.x, data.position.y,
						data.monster->get_container_used_slots());
			}
		}
	}

	// 列出所有 NPC
	if (npcs.size() > 0) {
		static const char *TYPE_NAMES[] = { "Villager", "Merchant", "QuestGiver", "Trainer", "Guard" };
		result += "NPCs:\n";
		for (int i = 0; i < npcs.size(); i++) {
			const NPCSpawnData &data = npcs[i];
			if (data.npc.is_valid()) {
				result += vformat("  [%d] %s (%s) Pos: (%d, %d), Items: %d\n",
						i, String(data.npc->get_character_name()),
						TYPE_NAMES[data.npc->get_npc_type()],
						data.position.x, data.position.y,
						data.npc->get_container_used_slots());
			}
		}
	}

	return result;
}

Ref<WorldObject> Chunk::get_city_object(int index) const {
	ERR_FAIL_INDEX_V(index, cities.size(), Ref<WorldObject>());
	return cities[index].world_object;
}

String Chunk::city_to_string(int index) const {
	ERR_FAIL_INDEX_V(index, cities.size(), String("Invalid city index"));

	const CityData &city = cities[index];
	Ref<WorldObject> obj = city.world_object;

	if (obj.is_null()) {
		return "City object is null";
	}

	String result;
	result += vformat("=== City WorldObject [%d] ===\n", index);
	result += vformat("  ID: %s\n", String(obj->get_object_id()));
	result += vformat("  Position: (%d, %d)\n", city.position.x, city.position.y);
	result += vformat("  Type: %d\n", obj->get_object_type());
	result += vformat("  Container: %d/%d slots used\n",
			obj->get_container_used_slots(), obj->get_container_capacity());

	result += "  Items:\n";
	for (int i = 0; i < obj->get_container_capacity(); i++) {
		Ref<Item> item = obj->container_get_item(i);
		if (item.is_valid() && !item->is_empty()) {
			static const char *RARITY_NAMES[] = { "Common", "Uncommon", "Rare", "Epic", "Legendary" };
			static const char *CATEGORY_NAMES[] = { "Material", "Consumable", "Equipment", "Tool", "Quest", "Misc" };

			result += vformat("    [Slot %d] %s x%d\n", i, String(item->get_item_id()), item->get_quantity());
			result += vformat("             Rarity: %s, Category: %s\n",
					RARITY_NAMES[item->get_rarity()], CATEGORY_NAMES[item->get_category()]);
		}
	}

	return result;
}

void Chunk::generate_monsters(RandomPCG &rng) {
	// 在 FOREST 和 MOUNTAIN 区域生成怪物
	Vector<Vector2i> spawn_positions;

	const int sample_step = 16;
	for (int y = 0; y < CHUNK_SIZE; y += sample_step) {
		for (int x = 0; x < CHUNK_SIZE; x += sample_step) {
			TileType type = tiles[y][x];
			if (type == TILE_FOREST || type == TILE_MOUNTAIN) {
				spawn_positions.push_back(Vector2i(x, y));
			}
		}
	}

	// 限制最大怪物数量
	const int max_monsters = 8;
	int num_monsters = MIN((int)spawn_positions.size(), max_monsters);

	for (int i = 0; i < num_monsters && spawn_positions.size() > 0; i++) {
		int idx = rng.rand(spawn_positions.size());
		Vector2i pos = spawn_positions[idx];
		spawn_positions.remove_at(idx);

		// 创建怪物
		Ref<Monster> monster;
		monster.instantiate();

		// 随机怪物类型
		int monster_idx = rng.rand(MONSTER_ID_COUNT);
		StringName monster_id = vformat("monster_%d_%d_%d_%d", coord.x, coord.y, pos.x, pos.y);
		monster->set_object_id(monster_id);
		monster->set_monster_id(StringName(MONSTER_IDS[monster_idx]));
		monster->set_character_name(StringName(MONSTER_IDS[monster_idx]));
		monster->set_local_position(pos);
		monster->set_spawn_position(pos);

		// 随机等级（基于地形）
		TileType terrain = tiles[pos.y][pos.x];
		MonsterRank rank;
		int rank_roll = rng.rand(100);
		if (terrain == TILE_MOUNTAIN) {
			// 山区有更高概率出精英/BOSS
			if (rank_roll < 60) {
				rank = MONSTER_RANK_NORMAL;
			} else if (rank_roll < 85) {
				rank = MONSTER_RANK_ELITE;
			} else if (rank_roll < 95) {
				rank = MONSTER_RANK_CHAMPION;
			} else {
				rank = MONSTER_RANK_BOSS;
			}
		} else {
			// 森林以普通怪为主
			if (rank_roll < 80) {
				rank = MONSTER_RANK_NORMAL;
			} else if (rank_roll < 95) {
				rank = MONSTER_RANK_ELITE;
			} else {
				rank = MONSTER_RANK_CHAMPION;
			}
		}
		monster->set_rank(rank);

		// 根据等级设置基础属性
		float multiplier = monster->get_rank_multiplier();
		int base_hp = 50 + rng.rand(50);
		monster->set_max_health((int)(base_hp * multiplier));
		monster->set_health(monster->get_max_health());

		// 设置奖励
		monster->set_exp_reward(10 + rng.rand(20));
		monster->set_gold_reward(5 + rng.rand(15));

		// 初始化背包（怪物也可以携带物品）
		int container_size = 2 + rng.rand(3);
		monster->init_container(container_size);

		// 随机添加 0-2 个物品（掉落物）
		int item_count = rng.rand(3);
		for (int j = 0; j < item_count; j++) {
			Ref<Item> item;
			item.instantiate();

			int item_idx = rng.rand(ITEM_COUNT);
			item->set_item_id(StringName(ITEM_IDS[item_idx]));
			item->set_display_name(StringName(ITEM_IDS[item_idx]));
			item->set_quantity(1 + rng.rand(5));
			item->set_max_stack_size(99);
			item->set_rarity(static_cast<ItemRarity>(rng.rand(5)));
			item->set_category(static_cast<ItemCategory>(rng.rand(6)));

			monster->container_add_item(item);
		}

		// 保存怪物数据
		MonsterSpawnData spawn_data;
		spawn_data.position = pos;
		spawn_data.monster = monster;
		monsters.push_back(spawn_data);
	}
}

void Chunk::generate_npcs(RandomPCG &rng) {
	// 在 CITY, TOWN, VILLAGE 区域生成 NPC
	Vector<Vector2i> spawn_positions;

	const int sample_step = 12;
	for (int y = 0; y < CHUNK_SIZE; y += sample_step) {
		for (int x = 0; x < CHUNK_SIZE; x += sample_step) {
			TileType type = tiles[y][x];
			if (type == TILE_CITY || type == TILE_TOWN || type == TILE_VILLAGE) {
				spawn_positions.push_back(Vector2i(x, y));
			}
		}
	}

	// 限制最大 NPC 数量
	const int max_npcs = 6;
	int num_npcs = MIN((int)spawn_positions.size(), max_npcs);

	for (int i = 0; i < num_npcs && spawn_positions.size() > 0; i++) {
		int idx = rng.rand(spawn_positions.size());
		Vector2i pos = spawn_positions[idx];
		spawn_positions.remove_at(idx);

		// 创建 NPC
		Ref<NPC> npc;
		npc.instantiate();

		// 随机 NPC 类型
		int npc_idx = rng.rand(NPC_ID_COUNT);
		StringName npc_id = vformat("npc_%d_%d_%d_%d", coord.x, coord.y, pos.x, pos.y);
		npc->set_object_id(npc_id);
		npc->set_character_name(StringName(NPC_IDS[npc_idx]));
		npc->set_local_position(pos);
		npc->set_spawn_position(pos);

		// 根据地形设置 NPC 类型
		TileType terrain = tiles[pos.y][pos.x];
		NPCType npc_type;
		if (terrain == TILE_CITY) {
			// 城市有更多商人和训练师
			int type_roll = rng.rand(5);
			npc_type = static_cast<NPCType>(type_roll);
		} else if (terrain == TILE_TOWN) {
			// 城镇以商人和村民为主
			int type_roll = rng.rand(100);
			if (type_roll < 40) {
				npc_type = NPC_TYPE_MERCHANT;
			} else if (type_roll < 70) {
				npc_type = NPC_TYPE_VILLAGER;
			} else {
				npc_type = NPC_TYPE_GUARD;
			}
		} else {
			// 村庄以村民为主
			npc_type = NPC_TYPE_VILLAGER;
		}
		npc->set_npc_type(npc_type);

		// 设置商人属性
		if (npc_type == NPC_TYPE_MERCHANT) {
			npc->set_is_merchant(true);
			npc->set_shop_id(vformat("shop_%s", NPC_IDS[npc_idx]));
		}

		// 设置对话
		npc->set_can_talk(true);
		npc->set_dialogue_id(vformat("dialogue_%s", NPC_IDS[npc_idx]));

		// 基础属性
		npc->set_max_health(100 + rng.rand(50));
		npc->set_health(npc->get_max_health());

		// 初始化背包（NPC 商人会有更多物品）
		int container_size = (npc_type == NPC_TYPE_MERCHANT) ? (6 + rng.rand(5)) : (2 + rng.rand(3));
		npc->init_container(container_size);

		// 商人随机添加 3-6 个物品，其他 NPC 0-2 个
		int item_count = (npc_type == NPC_TYPE_MERCHANT) ? (3 + rng.rand(4)) : rng.rand(3);
		for (int j = 0; j < item_count; j++) {
			Ref<Item> item;
			item.instantiate();

			int item_idx = rng.rand(ITEM_COUNT);
			item->set_item_id(StringName(ITEM_IDS[item_idx]));
			item->set_display_name(StringName(ITEM_IDS[item_idx]));
			item->set_quantity(1 + rng.rand(20));
			item->set_max_stack_size(99);
			item->set_rarity(static_cast<ItemRarity>(rng.rand(5)));
			item->set_category(static_cast<ItemCategory>(rng.rand(6)));

			// 商人物品设置价值
			if (npc_type == NPC_TYPE_MERCHANT) {
				item->set_base_value(10 + rng.rand(100));
			}

			npc->container_add_item(item);
		}

		// 保存 NPC 数据
		NPCSpawnData spawn_data;
		spawn_data.position = pos;
		spawn_data.npc = npc;
		npcs.push_back(spawn_data);
	}
}

Ref<Monster> Chunk::get_monster(int index) const {
	ERR_FAIL_INDEX_V(index, monsters.size(), Ref<Monster>());
	return monsters[index].monster;
}

String Chunk::monster_to_string(int index) const {
	ERR_FAIL_INDEX_V(index, monsters.size(), String("Invalid monster index"));

	const MonsterSpawnData &data = monsters[index];
	Ref<Monster> monster = data.monster;

	if (monster.is_null()) {
		return "Monster is null";
	}

	static const char *RANK_NAMES[] = { "Normal", "Elite", "Champion", "Boss", "WorldBoss" };

	String result;
	result += vformat("=== Monster [%d] ===\n", index);
	result += vformat("  ID: %s\n", String(monster->get_object_id()));
	result += vformat("  Monster Type: %s\n", String(monster->get_monster_id()));
	result += vformat("  Name: %s\n", String(monster->get_character_name()));
	result += vformat("  Position: (%d, %d)\n", data.position.x, data.position.y);
	result += vformat("  Rank: %s (x%.1f)\n", RANK_NAMES[monster->get_rank()], monster->get_rank_multiplier());
	result += vformat("  Health: %d/%d\n", monster->get_health(), monster->get_max_health());
	result += vformat("  Rewards: %d EXP, %d Gold\n", (int64_t)monster->get_exp_reward(), (int64_t)monster->get_gold_reward());
	result += vformat("  Container: %d/%d slots used\n",
			monster->get_container_used_slots(), monster->get_container_capacity());

	if (monster->get_container_used_slots() > 0) {
		result += "  Loot:\n";
		for (int i = 0; i < monster->get_container_capacity(); i++) {
			Ref<Item> item = monster->container_get_item(i);
			if (item.is_valid() && !item->is_empty()) {
				static const char *RARITY_NAMES[] = { "Common", "Uncommon", "Rare", "Epic", "Legendary" };
				result += vformat("    [Slot %d] %s x%d (%s)\n",
						i, String(item->get_item_id()), item->get_quantity(), RARITY_NAMES[item->get_rarity()]);
			}
		}
	}

	return result;
}

Ref<NPC> Chunk::get_npc(int index) const {
	ERR_FAIL_INDEX_V(index, npcs.size(), Ref<NPC>());
	return npcs[index].npc;
}

String Chunk::npc_to_string(int index) const {
	ERR_FAIL_INDEX_V(index, npcs.size(), String("Invalid NPC index"));

	const NPCSpawnData &data = npcs[index];
	Ref<NPC> npc = data.npc;

	if (npc.is_null()) {
		return "NPC is null";
	}

	static const char *TYPE_NAMES[] = { "Villager", "Merchant", "QuestGiver", "Trainer", "Guard" };

	String result;
	result += vformat("=== NPC [%d] ===\n", index);
	result += vformat("  ID: %s\n", String(npc->get_object_id()));
	result += vformat("  Name: %s\n", String(npc->get_character_name()));
	result += vformat("  Position: (%d, %d)\n", data.position.x, data.position.y);
	result += vformat("  Type: %s\n", TYPE_NAMES[npc->get_npc_type()]);
	result += vformat("  Health: %d/%d\n", npc->get_health(), npc->get_max_health());
	result += vformat("  Is Merchant: %s\n", npc->get_is_merchant() ? "Yes" : "No");
	if (npc->get_is_merchant()) {
		result += vformat("  Shop ID: %s\n", String(npc->get_shop_id()));
	}
	result += vformat("  Dialogue ID: %s\n", String(npc->get_dialogue_id()));
	result += vformat("  Container: %d/%d slots used\n",
			npc->get_container_used_slots(), npc->get_container_capacity());

	if (npc->get_container_used_slots() > 0) {
		result += "  Inventory:\n";
		for (int i = 0; i < npc->get_container_capacity(); i++) {
			Ref<Item> item = npc->container_get_item(i);
			if (item.is_valid() && !item->is_empty()) {
				static const char *RARITY_NAMES[] = { "Common", "Uncommon", "Rare", "Epic", "Legendary" };
				result += vformat("    [Slot %d] %s x%d (%s)",
						i, String(item->get_item_id()), item->get_quantity(), RARITY_NAMES[item->get_rarity()]);
				if (item->get_base_value() > 0) {
					result += vformat(" - Value: %d", item->get_base_value());
				}
				result += "\n";
			}
		}
	}

	return result;
}
