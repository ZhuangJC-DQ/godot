@tool
extends Node3D

const CHUNK_SIZE = 256
const TILE_SIZE = 1.0

## 随机物品的候选 type_id（容器类型）
const LOOT_CONTAINER_TYPES := [100, 101]  # 宝箱, 背包
## 容器内可能出现的物品 type_id
const LOOT_ITEM_TYPES := [1, 2, 3, 4, 5]  # 铁剑, 金币, 药水, 面包, 铁盾

## 地图物品被点击时发出
signal map_item_clicked(container_id: int)

var _map_items: Array = []   # 运行时生成的 MapItem 引用

@export_group("Chunk Grid")
@export var start_chunk_x: int = 0:
	set(value):
		start_chunk_x = value
		if Engine.is_editor_hint():
			_update_visualization()

@export var start_chunk_y: int = 0:
	set(value):
		start_chunk_y = value
		if Engine.is_editor_hint():
			_update_visualization()

@export var chunk_grid_size: int = 2:
	set(value):
		chunk_grid_size = max(1, value)
		if Engine.is_editor_hint():
			_update_visualization()

@export_group("Visualization")
@export var height_scale: float = 50.0:
	set(value):
		height_scale = value
		if Engine.is_editor_hint():
			_update_visualization()

@export var show_towns: bool = true:
	set(value):
		show_towns = value
		if Engine.is_editor_hint():
			_update_visualization()

@export var show_roads: bool = true:
	set(value):
		show_roads = value
		if Engine.is_editor_hint():
			_update_visualization()

@export var road_color: Color = Color(0.4, 0.35, 0.3):
	set(value):
		road_color = value
		if Engine.is_editor_hint():
			_update_visualization()

@export var town_marker_radius: float = 2.0:
	set(value):
		town_marker_radius = max(0.1, value)
		if Engine.is_editor_hint():
			_update_visualization()

@export var town_marker_height_offset: float = 2.0:
	set(value):
		town_marker_height_offset = value
		if Engine.is_editor_hint():
			_update_visualization()

@export var town_color_min: Color = Color(0.9, 0.2, 0.2)
@export var town_color_max: Color = Color(0.2, 1.0, 0.2)

@export var regenerate: bool = false:
	set(value):
		if value and Engine.is_editor_hint():
			_update_visualization()
		regenerate = false

@export_group("Terrain Generation")
@export var seed: int = 1337

@export_range(0.001, 0.1, 0.001) var noise_frequency: float = 0.005

@export_range(1, 8, 1) var noise_octaves: int = 3

@export_range(1.0, 4.0, 0.1) var noise_lacunarity: float = 2.0

@export_range(0.1, 1.0, 0.05) var noise_gain: float = 0.4

@export var use_terrain_curve: bool = true

@export var apply_terrain_params: bool = false:
	set(value):
		if value:
			_apply_terrain_params()
		apply_terrain_params = false

var world_manager: WorldManager

func _ready():
	if not Engine.is_editor_hint():
		# 运行时模式
		world_manager = get_node("../WorldManager")
		if world_manager == null:
			push_error("WorldManager not found!")
			return
		# 清除编辑器保存的旧子节点
		for child in get_children():
			remove_child(child)
			child.queue_free()
		_apply_terrain_params()
		print("=== MapVisualizer Started ===")
		_visualize_all_chunks()
	else:
		# 编辑器模式 - 初始化但不自动应用参数
		world_manager = get_node_or_null("../WorldManager")
		if world_manager:
			_apply_terrain_params()

func _apply_terrain_params():
	if not is_inside_tree():
		return
	
	world_manager = get_node_or_null("../WorldManager")
	if world_manager == null:
		return
	
	var time_start = Time.get_ticks_msec()
	
	# 使用批量更新方法，只清除一次chunks
	world_manager.update_all_params(seed, noise_frequency, noise_octaves,
									noise_lacunarity, noise_gain, use_terrain_curve)
	
	var time_end = Time.get_ticks_msec()
	print("[GD] Params applied & clear: %d ms" % (time_end - time_start))
	
	# 在编辑器模式下更新可视化
	if Engine.is_editor_hint():
		_update_visualization()

func _update_visualization():
	if not is_inside_tree():
		return
	
	var clear_start = Time.get_ticks_msec()
	# 清除旧的可视化
	for child in get_children():
		remove_child(child)
		child.queue_free()
	# 强制处理延迟删除队列
	await get_tree().process_frame
	var clear_end = Time.get_ticks_msec()
	print("[GD] Clear old vis: %d ms" % (clear_end - clear_start))
	
	# 获取或创建 WorldManager
	world_manager = get_node_or_null("../WorldManager")
	if world_manager == null:
		print("[ERROR] WorldManager not found")
		return
	
	print("\n=== VISUALIZING %dx%d Chunks (%d,%d) ===" % [chunk_grid_size, chunk_grid_size, start_chunk_x, start_chunk_y])
	_visualize_all_chunks()

func _visualize_all_chunks():
	var total_start = Time.get_ticks_msec()
	# 生成chunk网格
	for cy in range(chunk_grid_size):
		for cx in range(chunk_grid_size):
			var chunk_x = start_chunk_x + cx
			var chunk_y = start_chunk_y + cy
			visualize_chunk(chunk_x, chunk_y)
	var total_end = Time.get_ticks_msec()
	print("\n========== TOTAL: %d ms ==========\n" % (total_end - total_start))

func visualize_chunk(chunk_x: int, chunk_y: int):
	var time_start = Time.get_ticks_msec()
	
	# 获取 chunk 数据
	var time_before_get = Time.get_ticks_msec()
	var chunk_data = world_manager.get_chunk_data(chunk_x, chunk_y)
	var time_after_get = Time.get_ticks_msec()
	print("[GD] get_chunk_data(%d,%d): %d ms" % [chunk_x, chunk_y, time_after_get - time_before_get])
	
	if chunk_data.is_empty():
		print("[ERROR] Chunk (%d, %d) empty!" % [chunk_x, chunk_y])
		return
	
	# 检查城镇生成（Chunk 内泊松圆盘采样）
	var time_before_town = Time.get_ticks_msec()
	var town_count = world_manager.get_town_count(chunk_x, chunk_y)
	var time_after_town = Time.get_ticks_msec()
	
	if town_count > 0:
		var towns = world_manager.get_chunk_towns(chunk_x, chunk_y)
		print("[TOWN] ✓ Generated %d towns in Chunk(%d,%d) | Query time: %d ms" % [
			town_count, chunk_x, chunk_y,
			time_after_town - time_before_town
		])
		for i in range(towns.size()):
			var town = towns[i]
			print("  [%d] Tile(%d,%d) | Suitability: %.3f" % [
				i + 1, town.tile_x, town.tile_y, town.suitability
			])
			if show_towns:
				_create_town_marker(chunk_x, chunk_y, town)
	else:
		print("[TOWN] ✗ No towns in Chunk(%d,%d) | Check time: %d ms" % [
			chunk_x, chunk_y,
			time_after_town - time_before_town
		])
	
	# 检查道路生成
	var road_tiles = {}  # 用于快速查找道路tile
	if show_roads:
		var time_before_road = Time.get_ticks_msec()
		var road_count = world_manager.get_road_count(chunk_x, chunk_y)
		if road_count > 0:
			var roads = world_manager.get_chunk_roads(chunk_x, chunk_y)
			var total_tiles = 0
			for road in roads:
				if road.has("tiles"):
					var tiles = road["tiles"]
					total_tiles += tiles.size()
					for tile in tiles:
						var key = Vector2i(tile.x, tile.y)
						road_tiles[key] = true
			var time_after_road = Time.get_ticks_msec()
			print("[ROAD] ✓ Generated %d road segments (%d tiles) in Chunk(%d,%d) | Query time: %d ms" % [
				road_count, total_tiles, chunk_x, chunk_y,
				time_after_road - time_before_road
			])
		else:
			var time_after_road = Time.get_ticks_msec()
			print("[ROAD] ✗ No roads in Chunk(%d,%d) | Check time: %d ms" % [
				chunk_x, chunk_y,
				time_after_road - time_before_road
			])
	
	# 创建可视化网格（传递道路信息）
	var time_before_mesh = Time.get_ticks_msec()
	create_terrain_mesh(chunk_x, chunk_y, road_tiles)
	var time_after_mesh = Time.get_ticks_msec()
	print("[GD] create_mesh(%d,%d): %d ms" % [chunk_x, chunk_y, time_after_mesh - time_before_mesh])
	
	var time_end = Time.get_ticks_msec()
	print("[GD] TOTAL CHUNK(%d,%d): %d ms\n" % [chunk_x, chunk_y, time_end - time_start])

func _create_town_marker(chunk_x: int, chunk_y: int, town: Dictionary):
	if not town.has("tile_x") or not town.has("tile_y"):
		return

	var tile_x = int(town["tile_x"])
	var tile_y = int(town["tile_y"])
	var suitability = 0.0
	if town.has("suitability"):
		suitability = float(town["suitability"])

	var height = world_manager.get_tile_height(chunk_x, chunk_y, tile_x, tile_y)
	if height < 0:
		height = 0.5

	var world_x = chunk_x * CHUNK_SIZE * TILE_SIZE + tile_x * TILE_SIZE
	var world_z = chunk_y * CHUNK_SIZE * TILE_SIZE + tile_y * TILE_SIZE
	var world_y = height * height_scale + town_marker_height_offset

	# ── 运行时：生成可交互的 MapItem ──
	if not Engine.is_editor_hint():
		var MapItemScript = preload("res://map_item.gd")
		var rng := RandomNumberGenerator.new()
		rng.seed = hash(Vector2i(chunk_x * 256 + tile_x, chunk_y * 256 + tile_y))

		# 随机选择容器类型
		var type_id: int = LOOT_CONTAINER_TYPES[rng.randi() % LOOT_CONTAINER_TYPES.size()]
		var container_id: int = ItemManagerSingleton.create_item(type_id)

		# 随机填充 2-5 个物品
		var item_count := rng.randi_range(2, 5)
		for _i in range(item_count):
			var loot_type: int = LOOT_ITEM_TYPES[rng.randi() % LOOT_ITEM_TYPES.size()]
			var loot_id: int = ItemManagerSingleton.create_item(loot_type)
			# 随机堆叠
			if loot_type in [2, 3, 4]:  # 金币/药水/面包 可堆叠
				ItemManagerSingleton.item_manager.set_stack_count(loot_id, rng.randi_range(1, 10))
			ItemManagerSingleton.add_to_container(loot_id, container_id)

		var map_item: Area3D = MapItemScript.new()
		add_child(map_item)   # 先加入场景树，再 setup（避免 global_transform 警告）
		map_item.setup(container_id, Vector3(world_x, world_y, world_z), type_id)
		map_item.clicked.connect(_on_map_item_clicked)
		_map_items.append(map_item)
		print("[MapItem] Spawned %s at Chunk(%d,%d) Tile(%d,%d)" % [
			ItemManagerSingleton.get_item_data(container_id).get("name", "?"),
			chunk_x, chunk_y, tile_x, tile_y])
		return

	# ── 编辑器模式：保持原有球体标记 ──
	var marker = MeshInstance3D.new()
	marker.mesh = SphereMesh.new()
	marker.mesh.radius = town_marker_radius
	marker.mesh.height = town_marker_radius * 2.0

	var material = StandardMaterial3D.new()
	material.shading_mode = BaseMaterial3D.SHADING_MODE_PER_PIXEL
	material.albedo_color = town_color_min.lerp(town_color_max, clamp(suitability, 0.0, 1.0))
	material.emission_enabled = true
	material.emission = material.albedo_color
	material.emission_energy_multiplier = 0.5
	marker.set_surface_override_material(0, material)

	marker.transform.origin = Vector3(world_x, world_y, world_z)
	marker.name = "Town_%d_%d_%d_%d" % [chunk_x, chunk_y, tile_x, tile_y]
	add_child(marker)
	if Engine.is_editor_hint():
		marker.owner = get_tree().edited_scene_root

func _on_map_item_clicked(map_item: Area3D) -> void:
	var cid: int = map_item.container_id
	print("[MapVisualizer] Item clicked: container_id=%d" % cid)
	map_item_clicked.emit(cid)

func create_terrain_mesh(chunk_x: int, chunk_y: int, road_tiles: Dictionary = {}):
	# 降采样以提升性能 - 可以动态调整
	var sample_step = 8 # 从4增加到8，减少75%的顶点数
	var sampled_size = CHUNK_SIZE / sample_step
	
	# 计算chunk在世界空间中的起始坐标
	var chunk_world_x = chunk_x * CHUNK_SIZE * TILE_SIZE
	var chunk_world_z = chunk_y * CHUNK_SIZE * TILE_SIZE
	
	# 创建自定义网格
	var arrays = []
	arrays.resize(Mesh.ARRAY_MAX)
	
	var vertices = PackedVector3Array()
	var colors = PackedColorArray()
	var normals = PackedVector3Array()
	var uvs = PackedVector2Array()
	var indices = PackedInt32Array()
	
	# 生成顶点和颜色
	for y in range(sampled_size):
		for x in range(sampled_size):
			# 计算实际位置
			var tile_x = x * sample_step
			var tile_y = y * sample_step
			
			# 获取高度值
			var height = world_manager.get_tile_height(chunk_x, chunk_y, tile_x, tile_y)
			if height < 0:
				height = 0.5
			
			# 使用高度值生成3D地形（Y轴为高度）
			var pos = Vector3(
				chunk_world_x + tile_x * TILE_SIZE,
				height * height_scale, # 应用高度缩放
				chunk_world_z + tile_y * TILE_SIZE
			)
			vertices.append(pos)
			
			# 检查是否是道路tile（检查采样范围内是否有道路）
			var is_road = false
			if show_roads and not road_tiles.is_empty():
				# 检查采样区域内是否有道路tile
				for dy in range(sample_step):
					for dx in range(sample_step):
						var check_x = tile_x + dx
						var check_y = tile_y + dy
						if check_x < CHUNK_SIZE and check_y < CHUNK_SIZE:
							var key = Vector2i(check_x, check_y)
							if road_tiles.has(key):
								is_road = true
								break
					if is_road:
						break
			
			# 根据高度或道路生成颜色
			var color: Color
			if is_road:
				color = road_color
			else:
				color = _get_height_color(height)
			colors.append(color)
			
			uvs.append(Vector2(float(x) / sampled_size, float(y) / sampled_size))
	
	# 生成三角形索引
	for y in range(sampled_size - 1):
		for x in range(sampled_size - 1):
			var i = y * sampled_size + x
			
			# 第一个三角形
			indices.append(i)
			indices.append(i + sampled_size)
			indices.append(i + 1)
			
			# 第二个三角形
			indices.append(i + 1)
			indices.append(i + sampled_size)
			indices.append(i + sampled_size + 1)
	
	# 计算法线
	normals = _calculate_normals(vertices, indices, sampled_size)
	
	# 设置数组
	arrays[Mesh.ARRAY_VERTEX] = vertices
	arrays[Mesh.ARRAY_NORMAL] = normals
	arrays[Mesh.ARRAY_COLOR] = colors
	arrays[Mesh.ARRAY_TEX_UV] = uvs
	arrays[Mesh.ARRAY_INDEX] = indices
	
	# 创建网格
	var array_mesh = ArrayMesh.new()
	array_mesh.add_surface_from_arrays(Mesh.PRIMITIVE_TRIANGLES, arrays)
	
	# 创建材质
	var material = StandardMaterial3D.new()
	material.vertex_color_use_as_albedo = true
	material.shading_mode = BaseMaterial3D.SHADING_MODE_PER_VERTEX
	material.cull_mode = BaseMaterial3D.CULL_DISABLED # 双面显示
	
	# 创建 MeshInstance3D
	var mesh_instance = MeshInstance3D.new()
	mesh_instance.mesh = array_mesh
	mesh_instance.set_surface_override_material(0, material)
	mesh_instance.name = "Chunk_%d_%d" % [chunk_x, chunk_y]
	
	add_child(mesh_instance)
	if Engine.is_editor_hint():
		mesh_instance.owner = get_tree().edited_scene_root
	
	# 手动构建碰撞体 —— 直接从顶点/索引数据创建 ConcavePolygonShape3D
	var faces = PackedVector3Array()
	for idx in range(0, indices.size(), 3):
		faces.append(vertices[indices[idx]])
		faces.append(vertices[indices[idx + 1]])
		faces.append(vertices[indices[idx + 2]])
	
	var shape = ConcavePolygonShape3D.new()
	shape.backface_collision = true
	shape.set_faces(faces)
	
	var static_body = StaticBody3D.new()
	static_body.collision_layer = 1
	static_body.collision_mask = 1
	static_body.name = "Chunk_%d_%d_col" % [chunk_x, chunk_y]
	
	var col_shape = CollisionShape3D.new()
	col_shape.shape = shape
	static_body.add_child(col_shape)
	
	# 碰撞体作为 MapVisualizer 的直接子节点（而非 MeshInstance3D 的子节点）
	add_child(static_body)
	
	print("[Collision] Chunk(%d,%d): %d triangles, body added" % [chunk_x, chunk_y, faces.size() / 3])
	
	if Engine.is_editor_hint():
		static_body.owner = get_tree().edited_scene_root
		col_shape.owner = get_tree().edited_scene_root

func _get_height_color(height: float) -> Color:
	# 可以自定义配色方案，这里使用地形色
	if height < 0.2:
		return Color(0.1, 0.3, 0.6) # 深蓝（水）
	elif height < 0.4:
		return Color(0.8, 0.7, 0.4) # 沙滩
	elif height < 0.6:
		return Color(0.3, 0.7, 0.3) # 草地
	elif height < 0.8:
		return Color(0.2, 0.5, 0.2) # 森林
	else:
		return Color(0.9, 0.9, 0.9) # 雪山
	
# 计算顶点法线
func _calculate_normals(vertices: PackedVector3Array, indices: PackedInt32Array, grid_size: int) -> PackedVector3Array:
	var normals = PackedVector3Array()
	normals.resize(vertices.size())
	
	# 初始化法线为零
	for i in range(vertices.size()):
		normals[i] = Vector3.ZERO
	
	# 计算每个三角形的法线并累加到顶点
	for i in range(0, indices.size(), 3):
		var i0 = indices[i]
		var i1 = indices[i + 1]
		var i2 = indices[i + 2]
		
		var v0 = vertices[i0]
		var v1 = vertices[i1]
		var v2 = vertices[i2]
		
		var edge1 = v1 - v0
		var edge2 = v2 - v0
		var face_normal = edge1.cross(edge2).normalized()
		
		normals[i0] += face_normal
		normals[i1] += face_normal
		normals[i2] += face_normal
	
	# 归一化所有法线
	for i in range(normals.size()):
		normals[i] = normals[i].normalized()
	
	return normals
