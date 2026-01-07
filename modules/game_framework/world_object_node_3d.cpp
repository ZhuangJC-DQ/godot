/**************************************************************************/
/*  world_object_node_3d.cpp                                              */
/**************************************************************************/

#include "world_object_node_3d.h"

#include "scene/resources/3d/box_shape_3d.h"
#include "scene/resources/3d/primitive_meshes.h"
#include "scene/resources/material.h"

void WorldObjectNode3D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_world_object", "world_object"), &WorldObjectNode3D::set_world_object);
	ClassDB::bind_method(D_METHOD("get_world_object"), &WorldObjectNode3D::get_world_object);
	ClassDB::bind_method(D_METHOD("create_default_visual", "size", "color"), &WorldObjectNode3D::create_default_visual, DEFVAL(Vector3(1, 1, 1)), DEFVAL(Color(0.8, 0.6, 0.4)));

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "world_object", PROPERTY_HINT_RESOURCE_TYPE, "WorldObject"), "set_world_object", "get_world_object");
}

WorldObjectNode3D::WorldObjectNode3D() {
	// 创建网格实例
	mesh_instance = memnew(MeshInstance3D);
	add_child(mesh_instance, false, INTERNAL_MODE_FRONT);
	mesh_instance->set_name("Mesh");

	// 创建碰撞形状
	collision_shape = memnew(CollisionShape3D);
	add_child(collision_shape, false, INTERNAL_MODE_FRONT);
	collision_shape->set_name("CollisionShape");

	// 创建默认的碰撞形状（1x1x1的立方体）
	// 这样即使不调用 create_default_visual，对象也能被射线检测到
	Ref<BoxShape3D> default_shape;
	default_shape.instantiate();
	default_shape->set_size(Vector3(1, 1, 1));
	collision_shape->set_shape(default_shape);
}

WorldObjectNode3D::~WorldObjectNode3D() = default;

void WorldObjectNode3D::set_world_object(const Ref<WorldObject> &p_world_object) {
	world_object = p_world_object;
}

void WorldObjectNode3D::create_default_visual(const Vector3 &p_size, const Color &p_color) {
	if (!mesh_instance || !collision_shape) {
		return;
	}

	// 创建立方体网格
	Ref<BoxMesh> box_mesh;
	box_mesh.instantiate();
	box_mesh->set_size(p_size);
	mesh_instance->set_mesh(box_mesh);

	// 创建材质
	Ref<StandardMaterial3D> material;
	material.instantiate();
	material->set_albedo(p_color);
	material->set_roughness(0.7f);
	mesh_instance->set_surface_override_material(0, material);

	// 创建碰撞形状
	Ref<BoxShape3D> box_shape;
	box_shape.instantiate();
	box_shape->set_size(p_size);
	collision_shape->set_shape(box_shape);
}
