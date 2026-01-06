/**************************************************************************/
/*  game_framework.cpp                                                    */
/**************************************************************************/

#include "game_framework.h"

#include "core/config/engine.h"
#include "core/string/print_string.h"
#include "scene/3d/mesh_instance_3d.h"
#include "scene/3d/node_3d.h"
#include "scene/resources/3d/primitive_meshes.h"
#include "scene/resources/image_texture.h"
#include "scene/resources/material.h"

GameFramework::GameFramework() {
}

GameFramework::~GameFramework() {
}

void GameFramework::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_tile_size", "size"), &GameFramework::set_tile_size);
	ClassDB::bind_method(D_METHOD("get_tile_size"), &GameFramework::get_tile_size);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "tile_size", PROPERTY_HINT_RANGE, "0.1,10.0,0.1"), "set_tile_size", "get_tile_size");
}

void GameFramework::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
			_ready();
		} break;
	}
}

void GameFramework::_ready() {
	print_line("GameFramework: Ready!");
	test_map_generation();
	visualize_chunk(0, 0);
}

void GameFramework::test_map_generation() {
	print_line("\n========== MAP GENERATION TEST ==========\n");

	// 打印 chunk (0, 0)
	world.print_chunk(0, 0, 32);

	// 获取 chunk 并打印详细信息
	Chunk *chunk = world.get_chunk(0, 0);
	if (chunk) {
		// 打印第一个城市
		if (chunk->get_city_count() > 0) {
			print_line("\n========== CITY DETAILS ==========\n");
			print_line(chunk->city_to_string(0));
		}

		// 打印第一个怪物
		if (chunk->get_monster_count() > 0) {
			print_line("\n========== MONSTER DETAILS ==========\n");
			print_line(chunk->monster_to_string(0));
		}

		// 打印第一个 NPC
		if (chunk->get_npc_count() > 0) {
			print_line("\n========== NPC DETAILS ==========\n");
			print_line(chunk->npc_to_string(0));
		}
	}
}

void GameFramework::print_chunk_at(int32_t x, int32_t y) {
	world.print_chunk(x, y, 32);
}

void GameFramework::print_city(int32_t chunk_x, int32_t chunk_y, int32_t city_index) {
	Chunk *chunk = world.get_chunk(chunk_x, chunk_y);
	if (chunk && city_index < chunk->get_city_count()) {
		print_line(chunk->city_to_string(city_index));
	} else {
		print_line(vformat("City index %d not found in chunk (%d, %d)", city_index, chunk_x, chunk_y));
	}
}

void GameFramework::print_monster(int32_t chunk_x, int32_t chunk_y, int32_t monster_index) {
	Chunk *chunk = world.get_chunk(chunk_x, chunk_y);
	if (chunk && monster_index < chunk->get_monster_count()) {
		print_line(chunk->monster_to_string(monster_index));
	} else {
		print_line(vformat("Monster index %d not found in chunk (%d, %d)", monster_index, chunk_x, chunk_y));
	}
}

void GameFramework::print_npc(int32_t chunk_x, int32_t chunk_y, int32_t npc_index) {
	Chunk *chunk = world.get_chunk(chunk_x, chunk_y);
	if (chunk && npc_index < chunk->get_npc_count()) {
		print_line(chunk->npc_to_string(npc_index));
	} else {
		print_line(vformat("NPC index %d not found in chunk (%d, %d)", npc_index, chunk_x, chunk_y));
	}
}

void GameFramework::print_all_entities(int32_t chunk_x, int32_t chunk_y) {
	Chunk *chunk = world.get_chunk(chunk_x, chunk_y);
	if (!chunk) {
		print_line(vformat("Chunk (%d, %d) not found", chunk_x, chunk_y));
		return;
	}

	print_line(vformat("\n========== ALL ENTITIES IN CHUNK (%d, %d) ==========\n", chunk_x, chunk_y));

	// 打印所有城市
	print_line(vformat("--- CITIES (%d) ---\n", chunk->get_city_count()));
	for (int i = 0; i < chunk->get_city_count(); i++) {
		print_line(chunk->city_to_string(i));
	}

	// 打印所有怪物
	print_line(vformat("\n--- MONSTERS (%d) ---\n", chunk->get_monster_count()));
	for (int i = 0; i < chunk->get_monster_count(); i++) {
		print_line(chunk->monster_to_string(i));
	}

	// 打印所有 NPC
	print_line(vformat("\n--- NPCs (%d) ---\n", chunk->get_npc_count()));
	for (int i = 0; i < chunk->get_npc_count(); i++) {
		print_line(chunk->npc_to_string(i));
	}
}

// ============ 可视化方法 ============

Color GameFramework::get_tile_color(TileType type) const {
	switch (type) {
		case TILE_CITY:
			return Color(0.8f, 0.8f, 0.8f); // 灰色 - 城市
		case TILE_TOWN:
			return Color(0.6f, 0.5f, 0.4f); // 棕色 - 城镇
		case TILE_VILLAGE:
			return Color(0.9f, 0.8f, 0.6f); // 浅黄色 - 村庄
		case TILE_GRASSLAND:
			return Color(0.3f, 0.7f, 0.3f); // 绿色 - 草原
		case TILE_FOREST:
			return Color(0.1f, 0.4f, 0.1f); // 深绿色 - 森林
		case TILE_MOUNTAIN:
			return Color(0.5f, 0.5f, 0.5f); // 深灰色 - 山地
		default:
			return Color(1.0f, 0.0f, 1.0f); // 紫色 - 未知
	}
}

void GameFramework::visualize_chunk(int32_t chunk_x, int32_t chunk_y) {
	// 先清除旧的可视化节点（在获取 chunk 之前）
	clear_visualization();

	Chunk *chunk = world.get_chunk(chunk_x, chunk_y);
	if (!chunk) {
		print_line(vformat("Chunk (%d, %d) not found", chunk_x, chunk_y));
		return;
	}

	// 获取场景根节点用于设置 owner
	Node *scene_root = get_tree() ? get_tree()->get_edited_scene_root() : nullptr;
	if (!scene_root) {
		scene_root = get_owner();
	}
	if (!scene_root) {
		scene_root = this;
	}

	// 创建新的容器节点
	chunk_visual = memnew(Node3D);
	chunk_visual->set_name("ChunkVisual");
	add_child(chunk_visual);
	chunk_visual->set_owner(scene_root);

	// 创建地形网格
	create_terrain_mesh(chunk);

	// 创建实体可视化
	create_entity_visuals(chunk);

	// 设置所有子节点的 owner
	for (int i = 0; i < chunk_visual->get_child_count(); i++) {
		Node *child = chunk_visual->get_child(i);
		child->set_owner(scene_root);
	}

	print_line(vformat("Visualized chunk (%d, %d) with %d cities, %d monsters, %d npcs",
			chunk_x, chunk_y,
			chunk->get_city_count(),
			chunk->get_monster_count(),
			chunk->get_npc_count()));
}

void GameFramework::clear_visualization() {
	// 立即删除所有名为 ChunkVisual 的子节点（包括保存在场景中的）
	// 使用 memdelete 而不是 queue_free，确保立即删除
	for (int i = get_child_count() - 1; i >= 0; i--) {
		Node *child = get_child(i);
		String child_name = child->get_name();
		if (child_name.begins_with("ChunkVisual")) {
			remove_child(child);
			memdelete(child);
		}
	}
	chunk_visual = nullptr;
	// 注意：不要在这里调用 world.clear()，否则会导致数据丢失
}

void GameFramework::generate_in_editor() {
	// 在编辑器中生成可视化
	print_line("Generating visualization in editor...");
	visualize_chunk(0, 0);
}

void GameFramework::create_terrain_mesh(Chunk *chunk) {
	// 为了性能，我们按 tile type 分组创建材质
	// 每种地形类型使用一个平面

	// 先统计每种地形的格子
	const int sample_step = 4; // 采样步长，降低精度提升性能
	const int sampled_size = CHUNK_SIZE / sample_step;
	const float scaled_tile_size = tile_size * sample_step;

	// 创建地形图像纹理
	Ref<Image> terrain_image = Image::create_empty(sampled_size, sampled_size, false, Image::FORMAT_RGB8);

	for (int y = 0; y < sampled_size; y++) {
		for (int x = 0; x < sampled_size; x++) {
			int sample_x = x * sample_step;
			int sample_y = y * sample_step;
			TileType type = chunk->tiles[sample_y][sample_x];
			Color color = get_tile_color(type);
			terrain_image->set_pixel(x, y, color);
		}
	}

	// 创建纹理
	Ref<ImageTexture> terrain_texture = ImageTexture::create_from_image(terrain_image);

	// 创建平面网格
	Ref<PlaneMesh> plane_mesh;
	plane_mesh.instantiate();
	float plane_size = CHUNK_SIZE * tile_size;
	plane_mesh->set_size(Size2(plane_size, plane_size));

	// 创建材质
	Ref<StandardMaterial3D> material;
	material.instantiate();
	material->set_texture(StandardMaterial3D::TEXTURE_ALBEDO, terrain_texture);
	material->set_texture_filter(StandardMaterial3D::TEXTURE_FILTER_NEAREST); // 像素风格

	// 创建 MeshInstance3D
	MeshInstance3D *mesh_instance = memnew(MeshInstance3D);
	mesh_instance->set_name("TerrainMesh");
	mesh_instance->set_mesh(plane_mesh);
	mesh_instance->set_surface_override_material(0, material);

	// 设置位置（平面中心在 chunk 中心）
	float half_size = plane_size / 2.0f;
	mesh_instance->set_position(Vector3(half_size, 0, half_size));

	chunk_visual->add_child(mesh_instance);
}

void GameFramework::create_entity_visuals(Chunk *chunk) {
	// 创建城市可视化（正方体）
	for (int i = 0; i < chunk->cities.size(); i++) {
		const CityData &city = chunk->cities[i];

		MeshInstance3D *mesh_instance = memnew(MeshInstance3D);
		mesh_instance->set_name(vformat("City_%d", i));

		Ref<BoxMesh> box_mesh;
		box_mesh.instantiate();
		box_mesh->set_size(Vector3(4.0f, 4.0f, 4.0f));
		mesh_instance->set_mesh(box_mesh);

		// 创建材质（金色）
		Ref<StandardMaterial3D> material;
		material.instantiate();
		material->set_albedo(Color(1.0f, 0.84f, 0.0f)); // 金色
		mesh_instance->set_surface_override_material(0, material);

		// 设置位置
		float pos_x = city.position.x * tile_size;
		float pos_z = city.position.y * tile_size;
		mesh_instance->set_position(Vector3(pos_x, 2.0f, pos_z));

		chunk_visual->add_child(mesh_instance);
	}

	// 创建怪物可视化（胶囊体）
	for (int i = 0; i < chunk->monsters.size(); i++) {
		const MonsterSpawnData &monster_data = chunk->monsters[i];

		MeshInstance3D *mesh_instance = memnew(MeshInstance3D);
		mesh_instance->set_name(vformat("Monster_%d", i));

		Ref<CapsuleMesh> capsule_mesh;
		capsule_mesh.instantiate();
		capsule_mesh->set_radius(1.0f);
		capsule_mesh->set_height(3.0f);
		mesh_instance->set_mesh(capsule_mesh);

		// 根据怪物等级设置颜色
		Ref<StandardMaterial3D> material;
		material.instantiate();
		if (monster_data.monster.is_valid()) {
			switch (monster_data.monster->get_rank()) {
				case MONSTER_RANK_NORMAL:
					material->set_albedo(Color(1.0f, 0.0f, 0.0f)); // 红色
					break;
				case MONSTER_RANK_ELITE:
					material->set_albedo(Color(1.0f, 0.5f, 0.0f)); // 橙色
					break;
				case MONSTER_RANK_CHAMPION:
					material->set_albedo(Color(0.8f, 0.0f, 0.8f)); // 紫色
					break;
				case MONSTER_RANK_BOSS:
					material->set_albedo(Color(0.0f, 0.0f, 0.0f)); // 黑色
					break;
				default:
					material->set_albedo(Color(0.5f, 0.0f, 0.0f)); // 深红
					break;
			}
		} else {
			material->set_albedo(Color(1.0f, 0.0f, 0.0f));
		}
		mesh_instance->set_surface_override_material(0, material);

		// 设置位置
		float pos_x = monster_data.position.x * tile_size;
		float pos_z = monster_data.position.y * tile_size;
		mesh_instance->set_position(Vector3(pos_x, 1.5f, pos_z));

		chunk_visual->add_child(mesh_instance);
	}

	// 创建 NPC 可视化（圆锥/圆柱体）
	for (int i = 0; i < chunk->npcs.size(); i++) {
		const NPCSpawnData &npc_data = chunk->npcs[i];

		MeshInstance3D *mesh_instance = memnew(MeshInstance3D);
		mesh_instance->set_name(vformat("NPC_%d", i));

		// 使用圆柱体，顶部半径为0模拟圆锥
		Ref<CylinderMesh> cone_mesh;
		cone_mesh.instantiate();
		cone_mesh->set_top_radius(0.0f); // 顶部为0，形成圆锥
		cone_mesh->set_bottom_radius(1.0f);
		cone_mesh->set_height(3.0f);
		mesh_instance->set_mesh(cone_mesh);

		// 根据 NPC 类型设置颜色
		Ref<StandardMaterial3D> material;
		material.instantiate();
		if (npc_data.npc.is_valid()) {
			switch (npc_data.npc->get_npc_type()) {
				case NPC_TYPE_VILLAGER:
					material->set_albedo(Color(0.4f, 0.6f, 1.0f)); // 浅蓝
					break;
				case NPC_TYPE_MERCHANT:
					material->set_albedo(Color(0.0f, 1.0f, 0.0f)); // 绿色
					break;
				case NPC_TYPE_QUEST_GIVER:
					material->set_albedo(Color(1.0f, 1.0f, 0.0f)); // 黄色
					break;
				case NPC_TYPE_TRAINER:
					material->set_albedo(Color(0.0f, 1.0f, 1.0f)); // 青色
					break;
				case NPC_TYPE_GUARD:
					material->set_albedo(Color(0.5f, 0.5f, 1.0f)); // 蓝紫
					break;
				default:
					material->set_albedo(Color(0.0f, 0.5f, 1.0f)); // 蓝色
					break;
			}
		} else {
			material->set_albedo(Color(0.0f, 0.5f, 1.0f));
		}
		mesh_instance->set_surface_override_material(0, material);

		// 设置位置
		float pos_x = npc_data.position.x * tile_size;
		float pos_z = npc_data.position.y * tile_size;
		mesh_instance->set_position(Vector3(pos_x, 1.5f, pos_z));

		chunk_visual->add_child(mesh_instance);
	}
}
