/**************************************************************************/
/*  world_object_node_3d.h                                                */
/**************************************************************************/

#pragma once

#include "world_object.h"

#include "scene/3d/mesh_instance_3d.h"
#include "scene/3d/physics/static_body_3d.h"
#include "scene/3d/physics/collision_shape_3d.h"

// 用于在3D场景中表示WorldObject的节点
// 包含碰撞体，可以被射线检测到
class WorldObjectNode3D : public StaticBody3D {
	GDCLASS(WorldObjectNode3D, StaticBody3D);

private:
	Ref<WorldObject> world_object;  // 关联的WorldObject数据
	MeshInstance3D *mesh_instance = nullptr;
	CollisionShape3D *collision_shape = nullptr;

protected:
	static void _bind_methods();

public:
	WorldObjectNode3D();
	virtual ~WorldObjectNode3D();

	// 设置关联的WorldObject
	void set_world_object(const Ref<WorldObject> &p_world_object);
	Ref<WorldObject> get_world_object() const { return world_object; }

	// 创建基本的可视化表示（简单的立方体）
	void create_default_visual(const Vector3 &p_size = Vector3(1, 1, 1), const Color &p_color = Color(0.8, 0.6, 0.4));
};
