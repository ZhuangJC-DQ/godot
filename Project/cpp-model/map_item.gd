extends Area3D
## 地图上可交互的物品节点
## 在城镇位置生成，包含随机物品容器
## 玩家点击后发出 clicked 信号，打开容器 UI

const ITEM_COLORS := {
	100: Color(0.65, 0.45, 0.15),   # 宝箱 - 金棕色
	101: Color(0.45, 0.30, 0.15),   # 背包 - 深棕色
}
const DEFAULT_ITEM_COLOR := Color(0.5, 0.4, 0.2)

var container_id: int = -1
var _mesh_instance: MeshInstance3D
var _collision: CollisionShape3D

## 被玩家点击时发出
signal clicked(map_item: Area3D)

func _ready() -> void:
	# Area3D 需要开启物理拾取
	input_ray_pickable = true
	input_event.connect(_on_input_event)

	# 鼠标进入/离开高亮
	mouse_entered.connect(_on_mouse_entered)
	mouse_exited.connect(_on_mouse_exited)

## 初始化：传入容器物品 ID 和世界坐标
func setup(p_container_id: int, world_pos: Vector3, p_type_id: int = 100) -> void:
	container_id = p_container_id
	transform.origin = world_pos

	# 碰撞形状
	_collision = CollisionShape3D.new()
	var box := BoxShape3D.new()
	box.size = Vector3(2.5, 2.5, 2.5)
	_collision.shape = box
	add_child(_collision)

	# 网格 — 宝箱用 BoxMesh，背包用 SphereMesh
	_mesh_instance = MeshInstance3D.new()
	if p_type_id == 101:
		var sphere := SphereMesh.new()
		sphere.radius = 1.0
		sphere.height = 1.8
		_mesh_instance.mesh = sphere
	else:
		var box_mesh := BoxMesh.new()
		box_mesh.size = Vector3(2.0, 1.5, 1.5)
		_mesh_instance.mesh = box_mesh

	# 材质
	var mat := StandardMaterial3D.new()
	mat.albedo_color = ITEM_COLORS.get(p_type_id, DEFAULT_ITEM_COLOR)
	mat.emission_enabled = true
	mat.emission = mat.albedo_color
	mat.emission_energy_multiplier = 0.3
	_mesh_instance.set_surface_override_material(0, mat)
	add_child(_mesh_instance)

	# 名字标签
	var data := ItemManagerSingleton.get_item_data(p_container_id)
	name = "MapItem_%s_%d" % [data.get("name", "item"), p_container_id]

func _on_input_event(_camera: Node, event: InputEvent, _position: Vector3, _normal: Vector3, _shape_idx: int) -> void:
	if event is InputEventMouseButton:
		var mb := event as InputEventMouseButton
		if mb.button_index == MOUSE_BUTTON_LEFT and mb.pressed:
			clicked.emit(self)

func _on_mouse_entered() -> void:
	if _mesh_instance:
		var mat: StandardMaterial3D = _mesh_instance.get_surface_override_material(0)
		if mat:
			mat.emission_energy_multiplier = 1.2

func _on_mouse_exited() -> void:
	if _mesh_instance:
		var mat: StandardMaterial3D = _mesh_instance.get_surface_override_material(0)
		if mat:
			mat.emission_energy_multiplier = 0.3
