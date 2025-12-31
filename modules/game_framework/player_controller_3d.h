/**************************************************************************/
/*  player_controller_3d.h                                                */
/**************************************************************************/

#pragma once

#include "player.h"

#include "scene/3d/camera_3d.h"
#include "scene/3d/mesh_instance_3d.h"
#include "scene/3d/node_3d.h"
#include "scene/resources/3d/capsule_shape_3d.h"
#include "scene/resources/mesh.h"

// 第三人称玩家控制器 - 博德之门风格俯视角
class PlayerController3D : public Node3D {
	GDCLASS(PlayerController3D, Node3D);

private:
	// === 关联的 Player 数据 ===
	Ref<Player> player_data;

	// === 子节点 ===
	MeshInstance3D *mesh_instance = nullptr;
	Camera3D *camera = nullptr;
	Node3D *camera_pivot = nullptr; // 摄像机旋转支点

	// === 摄像机设置 ===
	float camera_distance = 15.0f;       // 摄像机距离
	float camera_distance_min = 5.0f;    // 最小距离
	float camera_distance_max = 30.0f;   // 最大距离
	float camera_zoom_speed = 2.0f;      // 缩放速度
	float camera_pitch = -60.0f;         // 俯仰角（度）博德之门风格
	float camera_yaw = 0.0f;             // 偏航角（度）
	float camera_rotation_speed = 0.3f;  // 旋转灵敏度

	// === 移动设置 ===
	float move_speed = 8.0f;             // 移动速度
	float rotation_speed = 10.0f;        // 角色旋转速度
	Vector3 velocity;                    // 当前速度
	Vector3 target_direction;            // 目标方向

	// === 输入状态 ===
	bool is_rotating_camera = false;     // 是否正在旋转摄像机（右键按住）
	Vector2 mouse_motion;                // 鼠标移动量

	// === 内部方法 ===
	void _setup_mesh();
	void _setup_camera();
	void _process_input(double p_delta);
	void _process_movement(double p_delta);
	void _process_camera(double p_delta);
	void _update_camera_position();

protected:
	static void _bind_methods();
	void _notification(int p_what);

	// 输入处理
	virtual void unhandled_input(const Ref<InputEvent> &p_event) override;

public:
	PlayerController3D();
	virtual ~PlayerController3D();

	// === Player 数据 ===
	void set_player_data(const Ref<Player> &p_player);
	Ref<Player> get_player_data() const { return player_data; }

	// === 摄像机设置 ===
	void set_camera_distance(float p_distance);
	float get_camera_distance() const { return camera_distance; }

	void set_camera_distance_min(float p_min) { camera_distance_min = MAX(1.0f, p_min); }
	float get_camera_distance_min() const { return camera_distance_min; }

	void set_camera_distance_max(float p_max) { camera_distance_max = MAX(camera_distance_min, p_max); }
	float get_camera_distance_max() const { return camera_distance_max; }

	void set_camera_zoom_speed(float p_speed) { camera_zoom_speed = MAX(0.1f, p_speed); }
	float get_camera_zoom_speed() const { return camera_zoom_speed; }

	void set_camera_pitch(float p_pitch);
	float get_camera_pitch() const { return camera_pitch; }

	void set_camera_yaw(float p_yaw) { camera_yaw = p_yaw; }
	float get_camera_yaw() const { return camera_yaw; }

	void set_camera_rotation_speed(float p_speed) { camera_rotation_speed = MAX(0.01f, p_speed); }
	float get_camera_rotation_speed() const { return camera_rotation_speed; }

	// === 移动设置 ===
	void set_move_speed(float p_speed) { move_speed = MAX(0.0f, p_speed); }
	float get_move_speed() const { return move_speed; }

	void set_rotation_speed(float p_speed) { rotation_speed = MAX(0.0f, p_speed); }
	float get_rotation_speed() const { return rotation_speed; }

	// === 获取子节点 ===
	Camera3D *get_camera() const { return camera; }
	MeshInstance3D *get_mesh_instance() const { return mesh_instance; }
};
