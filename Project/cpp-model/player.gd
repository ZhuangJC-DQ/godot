extends CharacterBody3D

## 第三人称玩家控制器
## WASD 移动，鼠标旋转视角，Shift 加速

@export_group("Movement")
@export var walk_speed: float = 8.0
@export var sprint_speed: float = 16.0
@export var acceleration: float = 10.0
@export var deceleration: float = 15.0
@export var gravity: float = 30.0
@export var jump_velocity: float = 8.0

@export_group("Camera")
@export var mouse_sensitivity: float = 0.002
@export var camera_distance: float = 10.0
@export var camera_min_pitch: float = -80.0
@export var camera_max_pitch: float = 60.0
@export var camera_height_offset: float = 1.5

var _camera_pivot: Node3D
var _camera_arm: SpringArm3D
var _camera: Camera3D
var _mesh: MeshInstance3D

var _camera_yaw: float = 0.0
var _camera_pitch: float = 0.0
var _mouse_captured: bool = false
var _physics_warmup: int = 0
var _world_manager: WorldManager
var _terrain_ready: bool = false
var _fall_check_count: int = 0

const CHUNK_SIZE_CONST = 256


func _ready() -> void:
	_camera_pivot = $CameraPivot
	_camera_arm = $CameraPivot/SpringArm3D
	_camera = $CameraPivot/SpringArm3D/Camera3D
	_mesh = $MeshInstance3D

	_camera_arm.spring_length = camera_distance
	_camera_pitch = deg_to_rad(-30.0)
	_apply_camera_rotation()

	Input.mouse_mode = Input.MOUSE_MODE_CAPTURED
	_mouse_captured = true

	# 获取 WorldManager 引用
	_world_manager = get_node_or_null("../WorldManager")

	# 直接用 WorldManager 计算地形高度来放置玩家
	_snap_to_terrain_direct()

	# 等待物理碰撞生效（每帧用射线检测来确认）
	_physics_warmup = 60  # 最多等 60 帧
	print("[Player] Waiting for physics warmup (max 60 frames)...")


func _unhandled_input(event: InputEvent) -> void:
	if event is InputEventMouseMotion and _mouse_captured:
		_camera_yaw -= event.relative.x * mouse_sensitivity
		_camera_pitch -= event.relative.y * mouse_sensitivity
		_camera_pitch = clampf(_camera_pitch, deg_to_rad(camera_min_pitch), deg_to_rad(camera_max_pitch))
		_apply_camera_rotation()

	if event is InputEventKey and event.pressed and event.keycode == KEY_ESCAPE:
		if _mouse_captured:
			Input.mouse_mode = Input.MOUSE_MODE_VISIBLE
			_mouse_captured = false
		else:
			Input.mouse_mode = Input.MOUSE_MODE_CAPTURED
			_mouse_captured = true


func _physics_process(delta: float) -> void:
	# 物理引擎预热：等待碰撞体注册完成，用射线检测确认
	if _physics_warmup > 0:
		_physics_warmup -= 1
		velocity = Vector3.ZERO
		# 保持在地形高度上（每帧重新定位防止下落）
		_snap_to_terrain_direct()
		
		# 每帧尝试射线检测，确认物理碰撞已就绪
		var space_state = get_world_3d().direct_space_state
		var from = global_position + Vector3(0, 10, 0)
		var to = global_position + Vector3(0, -50, 0)
		var query = PhysicsRayQueryParameters3D.create(from, to)
		query.exclude = [get_rid()]
		var result = space_state.intersect_ray(query)
		if result:
			global_position = result.position + Vector3(0, 0.1, 0)
			_physics_warmup = 0
			_terrain_ready = true
			print("[Player] Collision detected! Snapped to Y=%.1f (warmup ended early)" % global_position.y)
		elif _physics_warmup == 0:
			print("[Player] WARNING: Warmup exhausted, collision never detected. Using direct position.")
			_terrain_ready = true
		return

	# 安全网：如果玩家持续下落，用 WorldManager 拉回
	if _world_manager:
		var expected_y = _get_terrain_y_at(global_position.x, global_position.z)
		if global_position.y < expected_y - 5.0:
			_fall_check_count += 1
			if _fall_check_count > 3:
				print("[Player] Fell below terrain (Y=%.1f, terrain=%.1f), snapping back!" % [global_position.y, expected_y])
				global_position.y = expected_y + 1.0
				velocity = Vector3.ZERO
				_fall_check_count = 0
				return
		else:
			_fall_check_count = 0

	# 重力
	if not is_on_floor():
		velocity.y -= gravity * delta

	# 跳跃
	if Input.is_action_just_pressed("jump") and is_on_floor():
		velocity.y = jump_velocity

	# 获取输入方向 (相对于摄像机朝向)
	var input_dir := Vector2.ZERO
	if Input.is_action_pressed("move_forward"):
		input_dir.y -= 1.0
	if Input.is_action_pressed("move_backward"):
		input_dir.y += 1.0
	if Input.is_action_pressed("move_left"):
		input_dir.x -= 1.0
	if Input.is_action_pressed("move_right"):
		input_dir.x += 1.0
	input_dir = input_dir.normalized()

	# 计算移动方向（基于摄像机水平朝向）
	var cam_basis := Basis(Vector3.UP, _camera_yaw)
	var direction := cam_basis * Vector3(input_dir.x, 0.0, input_dir.y)

	# 速度（是否冲刺）
	var is_sprinting := Input.is_action_pressed("sprint")
	var target_speed := sprint_speed if is_sprinting else walk_speed

	# 平滑加速/减速
	if direction.length() > 0.01:
		velocity.x = lerpf(velocity.x, direction.x * target_speed, acceleration * delta)
		velocity.z = lerpf(velocity.z, direction.z * target_speed, acceleration * delta)

		var look_dir := Vector2(direction.x, direction.z)
		if look_dir.length() > 0.01:
			var target_angle := atan2(look_dir.x, look_dir.y)
			_mesh.rotation.y = lerp_angle(_mesh.rotation.y, target_angle, 10.0 * delta)
	else:
		velocity.x = lerpf(velocity.x, 0.0, deceleration * delta)
		velocity.z = lerpf(velocity.z, 0.0, deceleration * delta)

	move_and_slide()


## 获取指定世界坐标处的地形高度 Y 值
func _get_terrain_y_at(wx: float, wz: float) -> float:
	if not _world_manager:
		return 0.0
	var chunk_x := int(floorf(wx / CHUNK_SIZE_CONST))
	var chunk_y := int(floorf(wz / CHUNK_SIZE_CONST))
	var tile_x := int(wx) % CHUNK_SIZE_CONST
	var tile_y := int(wz) % CHUNK_SIZE_CONST
	if tile_x < 0:
		tile_x += CHUNK_SIZE_CONST
	if tile_y < 0:
		tile_y += CHUNK_SIZE_CONST
	var height: float = _world_manager.get_tile_height(chunk_x, chunk_y, tile_x, tile_y)
	if height < 0:
		height = 0.5
	return height * 50.0


## 直接通过 WorldManager 计算地形高度来放置玩家（不依赖物理引擎）
func _snap_to_terrain_direct() -> void:
	var terrain_y = _get_terrain_y_at(global_position.x, global_position.z)
	global_position.y = terrain_y + 1.0  # 略高于地面，留出胶囊体半高


func _apply_camera_rotation() -> void:
	if _camera_pivot:
		_camera_pivot.rotation = Vector3(_camera_pitch, _camera_yaw, 0.0)
