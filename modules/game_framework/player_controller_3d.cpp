/**************************************************************************/
/*  player_controller_3d.cpp                                              */
/**************************************************************************/

#include "player_controller_3d.h"

#include "core/config/engine.h"
#include "core/input/input.h"
#include "core/input/input_event.h"
#include "scene/main/viewport.h"
#include "scene/resources/3d/capsule_shape_3d.h"
#include "scene/resources/3d/primitive_meshes.h"
#include "scene/resources/material.h"

void PlayerController3D::_bind_methods() {
	// === Player 数据 ===
	ClassDB::bind_method(D_METHOD("set_player_data", "player"), &PlayerController3D::set_player_data);
	ClassDB::bind_method(D_METHOD("get_player_data"), &PlayerController3D::get_player_data);
	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "player_data", PROPERTY_HINT_RESOURCE_TYPE, "Player"), "set_player_data", "get_player_data");

	// === 摄像机设置 ===
	ClassDB::bind_method(D_METHOD("set_camera_distance", "distance"), &PlayerController3D::set_camera_distance);
	ClassDB::bind_method(D_METHOD("get_camera_distance"), &PlayerController3D::get_camera_distance);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "camera_distance", PROPERTY_HINT_RANGE, "1,50,0.1"), "set_camera_distance", "get_camera_distance");

	ClassDB::bind_method(D_METHOD("set_camera_distance_min", "min"), &PlayerController3D::set_camera_distance_min);
	ClassDB::bind_method(D_METHOD("get_camera_distance_min"), &PlayerController3D::get_camera_distance_min);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "camera_distance_min", PROPERTY_HINT_RANGE, "1,20,0.1"), "set_camera_distance_min", "get_camera_distance_min");

	ClassDB::bind_method(D_METHOD("set_camera_distance_max", "max"), &PlayerController3D::set_camera_distance_max);
	ClassDB::bind_method(D_METHOD("get_camera_distance_max"), &PlayerController3D::get_camera_distance_max);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "camera_distance_max", PROPERTY_HINT_RANGE, "10,100,0.1"), "set_camera_distance_max", "get_camera_distance_max");

	ClassDB::bind_method(D_METHOD("set_camera_zoom_speed", "speed"), &PlayerController3D::set_camera_zoom_speed);
	ClassDB::bind_method(D_METHOD("get_camera_zoom_speed"), &PlayerController3D::get_camera_zoom_speed);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "camera_zoom_speed", PROPERTY_HINT_RANGE, "0.1,10,0.1"), "set_camera_zoom_speed", "get_camera_zoom_speed");

	ClassDB::bind_method(D_METHOD("set_camera_pitch", "pitch"), &PlayerController3D::set_camera_pitch);
	ClassDB::bind_method(D_METHOD("get_camera_pitch"), &PlayerController3D::get_camera_pitch);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "camera_pitch", PROPERTY_HINT_RANGE, "-89,-10,0.1"), "set_camera_pitch", "get_camera_pitch");

	ClassDB::bind_method(D_METHOD("set_camera_yaw", "yaw"), &PlayerController3D::set_camera_yaw);
	ClassDB::bind_method(D_METHOD("get_camera_yaw"), &PlayerController3D::get_camera_yaw);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "camera_yaw", PROPERTY_HINT_RANGE, "-180,180,0.1"), "set_camera_yaw", "get_camera_yaw");

	ClassDB::bind_method(D_METHOD("set_camera_rotation_speed", "speed"), &PlayerController3D::set_camera_rotation_speed);
	ClassDB::bind_method(D_METHOD("get_camera_rotation_speed"), &PlayerController3D::get_camera_rotation_speed);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "camera_rotation_speed", PROPERTY_HINT_RANGE, "0.01,1,0.01"), "set_camera_rotation_speed", "get_camera_rotation_speed");

	// === 移动设置 ===
	ClassDB::bind_method(D_METHOD("set_move_speed", "speed"), &PlayerController3D::set_move_speed);
	ClassDB::bind_method(D_METHOD("get_move_speed"), &PlayerController3D::get_move_speed);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "move_speed", PROPERTY_HINT_RANGE, "0,50,0.1"), "set_move_speed", "get_move_speed");

	ClassDB::bind_method(D_METHOD("set_rotation_speed", "speed"), &PlayerController3D::set_rotation_speed);
	ClassDB::bind_method(D_METHOD("get_rotation_speed"), &PlayerController3D::get_rotation_speed);
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "rotation_speed", PROPERTY_HINT_RANGE, "0,20,0.1"), "set_rotation_speed", "get_rotation_speed");

	// === 获取子节点 ===
	ClassDB::bind_method(D_METHOD("get_camera"), &PlayerController3D::get_camera);
	ClassDB::bind_method(D_METHOD("get_mesh_instance"), &PlayerController3D::get_mesh_instance);
}

PlayerController3D::PlayerController3D() {
	// 创建默认 Player 数据
	player_data.instantiate();
}

PlayerController3D::~PlayerController3D() = default;

void PlayerController3D::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
			_setup_mesh();
			_setup_camera();
			_update_camera_position();
			set_process(true);
			set_process_unhandled_input(true);
		} break;

		case NOTIFICATION_PROCESS: {
			// 只在游戏运行时处理输入，编辑器模式下不处理
			if (Engine::get_singleton()->is_editor_hint()) {
				return;
			}
			double delta = get_process_delta_time();
			_process_input(delta);
			_process_movement(delta);
			_process_camera(delta);
		} break;
	}
}

void PlayerController3D::unhandled_input(const Ref<InputEvent> &p_event) {
	// 只在游戏运行时处理输入
	if (Engine::get_singleton()->is_editor_hint()) {
		return;
	}

	// 处理鼠标移动
	Ref<InputEventMouseMotion> mouse_motion_event = p_event;
	if (mouse_motion_event.is_valid()) {
		mouse_motion = mouse_motion_event->get_relative();
		return;
	}

	// 处理鼠标滚轮
	Ref<InputEventMouseButton> mouse_button_event = p_event;
	if (mouse_button_event.is_valid() && mouse_button_event->is_pressed()) {
		if (mouse_button_event->get_button_index() == MouseButton::WHEEL_UP) {
			set_camera_distance(camera_distance - camera_zoom_speed);
		} else if (mouse_button_event->get_button_index() == MouseButton::WHEEL_DOWN) {
			set_camera_distance(camera_distance + camera_zoom_speed);
		}
	}
}

void PlayerController3D::_setup_mesh() {
	// 创建角色网格（白色胶囊）
	mesh_instance = memnew(MeshInstance3D);
	add_child(mesh_instance);
	mesh_instance->set_name("CharacterMesh");

	// 创建胶囊网格
	Ref<CapsuleMesh> capsule_mesh;
	capsule_mesh.instantiate();
	capsule_mesh->set_radius(0.4f);
	capsule_mesh->set_height(1.8f);
	mesh_instance->set_mesh(capsule_mesh);

	// 创建白色材质
	Ref<StandardMaterial3D> material;
	material.instantiate();
	material->set_albedo(Color(1.0f, 1.0f, 1.0f, 1.0f)); // 白色
	material->set_roughness(0.8f);
	mesh_instance->set_surface_override_material(0, material);

	// 将胶囊稍微抬起，使底部在地面上
	mesh_instance->set_position(Vector3(0, 0.9f, 0));
}

void PlayerController3D::_setup_camera() {
	// 创建摄像机支点（用于旋转）
	camera_pivot = memnew(Node3D);
	add_child(camera_pivot);
	camera_pivot->set_name("CameraPivot");

	// 创建摄像机
	camera = memnew(Camera3D);
	camera_pivot->add_child(camera);
	camera->set_name("Camera");
	camera->set_current(true);
	camera->set_fov(60.0f);
	camera->set_far(1000.0f);
}

void PlayerController3D::_process_input(double p_delta) {
	Input *input = Input::get_singleton();
	if (!input) {
		return;
	}

	// 处理鼠标滚轮缩放
	// 注意：滚轮输入需要在 _input 中处理，这里处理按键

	// 右键按住状态检测
	is_rotating_camera = input->is_mouse_button_pressed(MouseButton::RIGHT);
}

void PlayerController3D::_process_movement(double p_delta) {
	Input *input = Input::get_singleton();
	if (!input) {
		return;
	}

	// 获取输入方向
	Vector2 input_dir;
	if (input->is_key_pressed(Key::W)) {
		input_dir.y += 1.0f;
	}
	if (input->is_key_pressed(Key::S)) {
		input_dir.y -= 1.0f;
	}
	if (input->is_key_pressed(Key::A)) {
		input_dir.x -= 1.0f;
	}
	if (input->is_key_pressed(Key::D)) {
		input_dir.x += 1.0f;
	}

	// 如果有输入
	if (input_dir.length_squared() > 0.0f) {
		input_dir = input_dir.normalized();

		// 根据摄像机朝向计算移动方向
		float yaw_rad = Math::deg_to_rad(camera_yaw);
		Vector3 forward = Vector3(Math::sin(yaw_rad), 0, Math::cos(yaw_rad));
		Vector3 right = Vector3(Math::cos(yaw_rad), 0, -Math::sin(yaw_rad));

		target_direction = (forward * -input_dir.y + right * input_dir.x).normalized();

		// 计算速度
		float current_speed = move_speed;
		if (player_data.is_valid()) {
			current_speed = player_data->get_move_speed();
		}
		velocity = target_direction * current_speed;

		// 角色模型朝向移动方向（只旋转模型，不旋转控制器本身）
		if (mesh_instance && target_direction.length_squared() > 0.01f) {
			float target_angle = Math::atan2(target_direction.x, target_direction.z);
			float current_angle = mesh_instance->get_rotation().y;

			// 平滑旋转
			float angle_diff = Math::angle_difference(current_angle, target_angle);
			float new_angle = current_angle - angle_diff * MIN(1.0f, rotation_speed * (float)p_delta);
			mesh_instance->set_rotation(Vector3(0, new_angle, 0));
		}
	} else {
		velocity = Vector3();
	}

	// 应用移动
	Vector3 pos = get_position();
	pos += velocity * (float)p_delta;
	set_position(pos);
}

void PlayerController3D::_process_camera(double p_delta) {
	Input *input = Input::get_singleton();
	if (!input) {
		return;
	}

	// 处理摄像机旋转（右键按住时）
	if (is_rotating_camera && mouse_motion.length_squared() > 0.0f) {
		camera_yaw -= mouse_motion.x * camera_rotation_speed;
		camera_pitch -= mouse_motion.y * camera_rotation_speed;

		// 限制俯仰角
		camera_pitch = CLAMP(camera_pitch, -89.0f, -10.0f);

		// 保持偏航角在 -180 到 180 之间
		while (camera_yaw > 180.0f) {
			camera_yaw -= 360.0f;
		}
		while (camera_yaw < -180.0f) {
			camera_yaw += 360.0f;
		}
	}

	// 重置鼠标移动量
	mouse_motion = Vector2();

	_update_camera_position();
}

void PlayerController3D::_update_camera_position() {
	if (!camera_pivot || !camera) {
		return;
	}

	// 设置支点旋转
	camera_pivot->set_rotation(Vector3(Math::deg_to_rad(camera_pitch), Math::deg_to_rad(camera_yaw), 0));

	// 摄像机位置（相对于支点）
	camera->set_position(Vector3(0, 0, camera_distance));

	// 摄像机始终看向角色位置（支点位置偏移一点高度）
	camera_pivot->set_position(Vector3(0, 1.0f, 0)); // 看向角色中心偏上
}

// === Player 数据 ===

void PlayerController3D::set_player_data(const Ref<Player> &p_player) {
	player_data = p_player;
}

// === 摄像机设置 ===

void PlayerController3D::set_camera_distance(float p_distance) {
	camera_distance = CLAMP(p_distance, camera_distance_min, camera_distance_max);
	_update_camera_position();
}

void PlayerController3D::set_camera_pitch(float p_pitch) {
	camera_pitch = CLAMP(p_pitch, -89.0f, -10.0f);
	_update_camera_position();
}
