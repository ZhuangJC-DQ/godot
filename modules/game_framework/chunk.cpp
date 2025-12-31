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

	// 清空城市列表
	cities.clear();

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
	result += vformat("=== Chunk (%d, %d) | Center: (%d, %d) | Cities: %d ===\n",
			coord.x, coord.y, center_x, center_y, cities.size());
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
		result += "Cities in this chunk:\n";
		for (int i = 0; i < cities.size(); i++) {
			const CityData &city = cities[i];
			result += vformat("  [%d] Position: (%d, %d), Items: %d\n",
					i, city.position.x, city.position.y,
					city.world_object.is_valid() ? city.world_object->get_container_used_slots() : 0);
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
