extends Node3D

const CHUNK_SIZE = 256
const TILE_SIZE = 1.0

var world_manager: WorldManager

# 地形颜色映射
var tile_colors = {
	0: Color(0.3, 0.7, 0.3),  # TILE_GRASSLAND - 绿色
	1: Color(0.1, 0.4, 0.1),  # TILE_FOREST - 深绿色
	2: Color(0.5, 0.5, 0.5),  # TILE_MOUNTAIN - 深灰色
	3: Color(0.8, 0.8, 0.8),  # TILE_CITY - 灰色
	4: Color(0.6, 0.5, 0.4),  # TILE_TOWN - 棕色
	5: Color(0.9, 0.8, 0.6),  # TILE_VILLAGE - 浅黄色
}

func _ready():
	# 获取 WorldManager 节点
	world_manager = get_node("../WorldManager")
	
	if world_manager == null:
		push_error("WorldManager not found!")
		return
	
	print("=== MapVisualizer Started ===")
	
	# 获取并打印 chunk (0, 0) 的数据
	visualize_chunk(0, 0)

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
	print("\n--- Creating Height Map Visualization ---\n")
	
	# 降采样以提升性能
	var sample_step = 4
	var sampled_size = CHUNK_SIZE / sample_step
	
	# 创建自定义网格
	var arrays = []
	arrays.resize(Mesh.ARRAY_MAX)
	
	var vertices = PackedVector3Array()
	var colors = PackedColorArray()
	var uvs = PackedVector2Array()
	var indices = PackedInt32Array()
	
	# 生成顶点和颜色
	for y in range(sampled_size):
		for x in range(sampled_size):
			# 计算实际位置
			var tile_x = x * sample_step
			var tile_y = y * sample_step
			
			# 获取高度值并转换为灰度颜色
			var height = world_manager.get_tile_height(chunk_x, chunk_y, tile_x, tile_y)
			if height < 0:
				height = 0.5
			var color = Color(height, height, height)
			
			# 添加顶点
			var pos = Vector3(x * TILE_SIZE * sample_step, 0, y * TILE_SIZE * sample_step)
			vertices.append(pos)
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
	
	# 设置数组
	arrays[Mesh.ARRAY_VERTEX] = vertices
	arrays[Mesh.ARRAY_COLOR] = colors
	arrays[Mesh.ARRAY_TEX_UV] = uvs
	arrays[Mesh.ARRAY_INDEX] = indices
	
	# 创建网格
	var array_mesh = ArrayMesh.new()
	array_mesh.add_surface_from_arrays(Mesh.PRIMITIVE_TRIANGLES, arrays)
	
	# 创建材质
	var material = StandardMaterial3D.new()
	material.vertex_color_use_as_albedo = true
	material.shading_mode = BaseMaterial3D.SHADING_MODE_UNSHADED
	
	# 创建 MeshInstance3D
	var mesh_instance = MeshInstance3D.new()
	mesh_instance.mesh = array_mesh
	mesh_instance.set_surface_override_material(0, material)
	mesh_instance.name = "Chunk_%d_%d" % [chunk_x, chunk_y]
	
	add_child(mesh_instance)
	if Engine.is_editor_hint():
		mesh_instance.owner = get_tree().edited_scene_root
	
	print("Created height map visualization with %d vertices" % vertices.size())
	print("Height values displayed as grayscale (0.0=black, 1.0=white)")
