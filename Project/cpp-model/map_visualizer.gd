@tool
extends Node3D

const CHUNK_SIZE = 256
const TILE_SIZE = 1.0

@export var chunk_x: int = 0:
	set(value):
		chunk_x = value
		if Engine.is_editor_hint():
			_update_visualization()

@export var chunk_y: int = 0:
	set(value):
		chunk_y = value
		if Engine.is_editor_hint():
			_update_visualization()

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

var world_manager: WorldManager

func _ready():
	if not Engine.is_editor_hint():
		# 运行时模式
		world_manager = get_node("../WorldManager")
		if world_manager == null:
			push_error("WorldManager not found!")
			return
		print("=== MapVisualizer Started ===")
		visualize_chunk(chunk_x, chunk_y)
	else:
		# 编辑器模式
		_update_visualization()

func _update_visualization():
	if not is_inside_tree():
		return
	
	# 清除旧的可视化
	for child in get_children():
		child.queue_free()
	
	# 获取或创建 WorldManager
	world_manager = get_node_or_null("../WorldManager")
	if world_manager == null:
		print("WorldManager not found in editor mode")
		return
	
	print("=== Editor Visualization: Chunk (%d, %d) ===" % [chunk_x, chunk_y])
	visualize_chunk(chunk_x, chunk_y)

func visualize_chunk(chunk_x: int, chunk_y: int):
	print("\n========== CHUNK (%d, %d) DATA ==========\n" % [chunk_x, chunk_y])
	
	# 获取 chunk 数据
	var chunk_data = world_manager.get_chunk_data(chunk_x, chunk_y)
	
	if chunk_data.is_empty():
		print("Chunk (%d, %d) not found or empty!" % [chunk_x, chunk_y])
		return
	
	# 打印 chunk 基本信息
	print("Chunk Coordinates: (%d, %d)" % [chunk_data["coord_x"], chunk_data["coord_y"]])
	print("Terrain Generated: Perlin Noise")
	
	# 统计高度分布
	var height_min = 1.0
	var height_max = 0.0
	var height_sum = 0.0
	
	# 采样高度数据
	var sample_step = 32
	var sample_count = 0
	print("\n--- Height Map Sample (every %d tiles) ---\n" % sample_step)
	
	for y in range(0, CHUNK_SIZE, sample_step):
		for x in range(0, CHUNK_SIZE, sample_step):
			var height = world_manager.get_tile_height(chunk_x, chunk_y, x, y)
			
			if height >= 0:
				height_min = min(height_min, height)
				height_max = max(height_max, height)
				height_sum += height
				sample_count += 1
			
				# 打印采样点
				if x % 64 == 0 and y % 64 == 0:
					print("  Tile (%d, %d): Height %.3f" % [x, y, height])
	
	# 打印高度统计
	print("\n--- Height Map Statistics ---\n")
	if sample_count > 0:
		var height_avg = height_sum / sample_count
		print("  Min Height: %.3f" % height_min)
		print("  Max Height: %.3f" % height_max)
		print("  Avg Height: %.3f" % height_avg)
		print("  Samples: %d" % sample_count)
	
	# 创建可视化网格
	create_terrain_mesh(chunk_x, chunk_y)
	
	print("\n========== END CHUNK DATA ==========\n")

func create_terrain_mesh(chunk_x: int, chunk_y: int):
	print("\n--- Creating 3D Height Map Terrain ---\n")
	
	# 降采样以提升性能
	var sample_step = 4
	var sampled_size = CHUNK_SIZE / sample_step
	
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
				x * TILE_SIZE * sample_step,
				height * height_scale,  # 应用高度缩放
				y * TILE_SIZE * sample_step
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
	
	print("Created 3D terrain with %d vertices" % vertices.size())
	print("Height scale: %.1f units" % height_scale)

# 根据高度生成颜色
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
