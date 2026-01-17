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
	print("Center Position: (%d, %d)" % [chunk_data["center_x"], chunk_data["center_y"]])
	
	# 统计地形类型分布
	var tile_type_counts = {}
	
	# 采样地形数据（每隔几格采样一次以减少输出）
	var sample_step = 32
	print("\n--- Terrain Sample (every %d tiles) ---\n" % sample_step)
	
	for y in range(0, CHUNK_SIZE, sample_step):
		for x in range(0, CHUNK_SIZE, sample_step):
			var tile_type = world_manager.get_tile_type(chunk_x, chunk_y, x, y)
			
			if not tile_type_counts.has(tile_type):
				tile_type_counts[tile_type] = 0
			tile_type_counts[tile_type] += 1
			
			# 打印采样点
			if x % 64 == 0 and y % 64 == 0:
				var type_name = get_tile_type_name(tile_type)
				print("  Tile (%d, %d): %s (Type %d)" % [x, y, type_name, tile_type])
	
	# 打印地形类型统计
	print("\n--- Terrain Type Distribution (Sampled) ---\n")
	for tile_type in tile_type_counts.keys():
		var count = tile_type_counts[tile_type]
		var type_name = get_tile_type_name(tile_type)
		var total_samples = (CHUNK_SIZE / sample_step) * (CHUNK_SIZE / sample_step)
		var percentage = (count * 100) / total_samples
		print("  %s: %d tiles (~%d%%)" % [type_name, count, percentage])
	
	# 创建可视化网格
	create_terrain_mesh(chunk_x, chunk_y)
	
	print("\n========== END CHUNK DATA ==========\n")

func create_terrain_mesh(chunk_x: int, chunk_y: int):
	print("\n--- Creating Terrain Visualization ---\n")
	
	# 降采样以提升性能
	var sample_step = 4
	var sampled_size = CHUNK_SIZE / sample_step
	
	# 创建平面网格
	var plane_mesh = PlaneMesh.new()
	var plane_size = CHUNK_SIZE * TILE_SIZE
	plane_mesh.size = Vector2(plane_size, plane_size)
	plane_mesh.subdivide_width = sampled_size - 1
	plane_mesh.subdivide_depth = sampled_size - 1
	
	# 创建材质
	var material = StandardMaterial3D.new()
	material.vertex_color_use_as_albedo = true
	
	# 创建 MeshInstance3D
	var mesh_instance = MeshInstance3D.new()
	mesh_instance.mesh = plane_mesh
	mesh_instance.material_override = material
	mesh_instance.name = "Chunk_%d_%d" % [chunk_x, chunk_y]
	
	# 设置位置
	var half_size = plane_size / 2.0
	mesh_instance.position = Vector3(half_size, 0, half_size)
	
	add_child(mesh_instance)
	
	print("Created terrain mesh at position %s" % str(mesh_instance.position))
	print("Mesh size: %.1f x %.1f (subdivisions: %dx%d)" % [plane_size, plane_size, sampled_size, sampled_size])

func get_tile_type_name(tile_type: int) -> String:
	match tile_type:
		0: return "GRASSLAND"
		1: return "FOREST"
		2: return "MOUNTAIN"
		3: return "CITY"
		4: return "TOWN"
		5: return "VILLAGE"
		_: return "UNKNOWN"
