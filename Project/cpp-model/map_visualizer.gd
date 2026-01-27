@tool
extends Node3D

const CHUNK_SIZE = 256
const TILE_SIZE = 1.0

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
	
	# 创建可视化网格
	var time_before_mesh = Time.get_ticks_msec()
	create_terrain_mesh(chunk_x, chunk_y)
	var time_after_mesh = Time.get_ticks_msec()
	print("[GD] create_mesh(%d,%d): %d ms" % [chunk_x, chunk_y, time_after_mesh - time_before_mesh])
	
	var time_end = Time.get_ticks_msec()
	print("[GD] TOTAL CHUNK(%d,%d): %d ms\n" % [chunk_x, chunk_y, time_end - time_start])

func create_terrain_mesh(chunk_x: int, chunk_y: int):
	# 降采样以提升性能 - 可以动态调整
	var sample_step = 8  # 从4增加到8，减少75%的顶点数
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
				height * height_scale,  # 应用高度缩放
				chunk_world_z + tile_y * TILE_SIZE
			)
			vertices.append(pos)
			
			# 根据高度生成颜色渐变（可选：也可以用其他配色方案）
			var color = _get_height_color(height)
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
	material.cull_mode = BaseMaterial3D.CULL_DISABLED  # 双面显示
	
	# 创建 MeshInstance3D
	var mesh_instance = MeshInstance3D.new()
	mesh_instance.mesh = array_mesh
	mesh_instance.set_surface_override_material(0, material)
	mesh_instance.name = "Chunk_%d_%d" % [chunk_x, chunk_y]
	
	add_child(mesh_instance)
	if Engine.is_editor_hint():
		mesh_instance.owner = get_tree().edited_scene_root

func _get_height_color(height: float) -> Color:
	# 可以自定义配色方案，这里使用地形色
	if height < 0.2:
		return Color(0.1, 0.3, 0.6)  # 深蓝（水）
	elif height < 0.4:
		return Color(0.8, 0.7, 0.4)  # 沙滩
	elif height < 0.6:
		return Color(0.3, 0.7, 0.3)  # 草地
	elif height < 0.8:
		return Color(0.2, 0.5, 0.2)  # 森林
	else:
		return Color(0.9, 0.9, 0.9)  # 雪山
	
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
