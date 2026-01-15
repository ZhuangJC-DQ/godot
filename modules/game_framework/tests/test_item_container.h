/**************************************************************************/
/*  test_item_container.h                                                 */
/**************************************************************************/

#pragma once

#include "tests/test_macros.h"

#include "modules/game_framework/item_container.h"
#include "modules/game_framework/world_object.h"
#include "modules/game_framework/item.h"

namespace TestItemContainer {

// ============ 辅助函数：创建测试对象 ============

Ref<WorldObject> create_test_object(const String &p_id) {
	Ref<WorldObject> obj;
	obj.instantiate();
	obj->setup(p_id, WorldObject::TYPE_GENERIC, Vector2i(0, 0));
	return obj;
}

Ref<Item> create_test_item(const String &p_id) {
	Ref<Item> item;
	item.instantiate();
	item->set_item_id(p_id);
	item->set_display_name(p_id);
	item->set_max_stack_size(1);
	return item;
}

Ref<WorldObject> create_container_object(const String &p_id, int32_t p_capacity) {
	Ref<WorldObject> obj = create_test_object(p_id);
	obj->init_container(p_capacity, 0);
	return obj;
}

// ============ 基础功能测试 ============

TEST_CASE("[ItemContainer] 初始化和容量管理") {
	Ref<ItemContainer> container;
	container.instantiate();

	SUBCASE("初始化容器") {
		container->initialize(10, 0);
		CHECK(container->get_capacity() == 10);
		CHECK(container->get_used_slots() == 0);
		CHECK(container->get_empty_slots() == 10);
		CHECK(container->is_empty());
		CHECK_FALSE(container->is_full());
	}

	SUBCASE("空容器") {
		container->initialize(0, 0);
		CHECK(container->get_capacity() == 0);
		CHECK(container->is_empty());
		CHECK(container->is_full());
	}
}

TEST_CASE("[ItemContainer] 添加和移除对象") {
	Ref<ItemContainer> container;
	container.instantiate();
	container->initialize(5, 0);

	Ref<WorldObject> obj1 = create_test_object("sword");
	Ref<WorldObject> obj2 = create_test_object("shield");

	SUBCASE("添加对象到第一个可用槽位") {
		CHECK(container->add_object(obj1));
		CHECK(container->get_used_slots() == 1);
		CHECK(container->get_object(0) == obj1);
	}

	SUBCASE("添加对象到指定槽位") {
		CHECK(container->add_object_at(2, obj1));
		CHECK(container->get_object(2) == obj1);
		CHECK(container->get_used_slots() == 1);
	}

	SUBCASE("添加到已占用的槽位应失败") {
		container->add_object_at(1, obj1);
		CHECK_FALSE(container->add_object_at(1, obj2));
		CHECK(container->get_object(1) == obj1);
	}

	SUBCASE("移除对象") {
		container->add_object(obj1);
		Ref<WorldObject> removed = container->remove_object(0);
		CHECK(removed == obj1);
		CHECK(container->is_empty());
	}

	SUBCASE("移除空槽位返回null") {
		Ref<WorldObject> removed = container->remove_object(0);
		CHECK(removed.is_null());
	}
}

TEST_CASE("[ItemContainer] 容器已满处理") {
	Ref<ItemContainer> container;
	container.instantiate();
	container->initialize(2, 0);

	Ref<WorldObject> obj1 = create_test_object("item1");
	Ref<WorldObject> obj2 = create_test_object("item2");
	Ref<WorldObject> obj3 = create_test_object("item3");

	container->add_object(obj1);
	container->add_object(obj2);

	CHECK(container->is_full());
	CHECK_FALSE(container->add_object(obj3));
	CHECK(container->get_used_slots() == 2);
}

TEST_CASE("[ItemContainer] 交换操作（防止复制BUG）") {
	Ref<ItemContainer> container;
	container.instantiate();
	container->initialize(5, 0);

	Ref<WorldObject> sword = create_test_object("sword");
	Ref<WorldObject> shield = create_test_object("shield");

	container->add_object_at(0, sword);
	container->add_object_at(2, shield);

	SUBCASE("交换两个有物品的槽位") {
		CHECK(container->swap_objects(0, 2));
		CHECK(container->get_object(0) == shield);
		CHECK(container->get_object(2) == sword);
		CHECK(container->get_used_slots() == 2); // 关键：物品数量不变
	}

	SUBCASE("交换一个有物品和一个空槽位") {
		CHECK(container->swap_objects(0, 1));
		CHECK(container->get_object(0).is_null());
		CHECK(container->get_object(1) == sword);
		CHECK(container->get_used_slots() == 2);
	}

	SUBCASE("交换相同槽位（无操作）") {
		CHECK(container->swap_objects(0, 0));
		CHECK(container->get_object(0) == sword);
	}

	SUBCASE("交换两个空槽位（无操作）") {
		CHECK(container->swap_objects(1, 3));
		CHECK(container->get_used_slots() == 2);
	}
}

TEST_CASE("[ItemContainer] 移动操作（原子性）") {
	Ref<ItemContainer> container;
	container.instantiate();
	container->initialize(5, 0);

	Ref<WorldObject> sword = create_test_object("sword");
	Ref<WorldObject> shield = create_test_object("shield");

	container->add_object_at(0, sword);

	SUBCASE("移动到空槽位成功") {
		CHECK(container->move_object(0, 2));
		CHECK(container->get_object(0).is_null());
		CHECK(container->get_object(2) == sword);
		CHECK(container->get_used_slots() == 1); // 关键：物品数量不变
	}

	SUBCASE("移动到已占用槽位失败") {
		container->add_object_at(1, shield);
		ERR_PRINT_OFF; // 禁用错误打印
		CHECK_FALSE(container->move_object(0, 1));
		ERR_PRINT_ON;
		CHECK(container->get_object(0) == sword);
		CHECK(container->get_object(1) == shield);
	}

	SUBCASE("从空槽位移动失败") {
		ERR_PRINT_OFF;
		CHECK_FALSE(container->move_object(3, 4));
		ERR_PRINT_ON;
	}
}

TEST_CASE("[ItemContainer] set_object 防止意外覆盖") {
	Ref<ItemContainer> container;
	container.instantiate();
	container->initialize(5, 0);

	Ref<WorldObject> sword = create_test_object("sword");
	Ref<WorldObject> shield = create_test_object("shield");

	SUBCASE("设置空槽位成功") {
		CHECK(container->set_object(0, sword));
		CHECK(container->get_object(0) == sword);
	}

	SUBCASE("设置已占用槽位失败") {
		container->add_object_at(0, sword);
		ERR_PRINT_OFF;
		CHECK_FALSE(container->set_object(0, shield));
		ERR_PRINT_ON;
		CHECK(container->get_object(0) == sword); // 原物品未被覆盖
	}
}

TEST_CASE("[ItemContainer] replace_object 强制替换") {
	Ref<ItemContainer> container;
	container.instantiate();
	container->initialize(5, 0);

	Ref<WorldObject> sword = create_test_object("sword");
	Ref<WorldObject> shield = create_test_object("shield");

	container->add_object_at(0, sword);

	SUBCASE("强制替换已有物品") {
		CHECK(container->replace_object(0, shield));
		CHECK(container->get_object(0) == shield);
		CHECK(container->get_used_slots() == 1);
	}

	SUBCASE("替换空槽位") {
		CHECK(container->replace_object(1, sword));
		CHECK(container->get_object(1) == sword);
	}

	SUBCASE("替换为null清空槽位") {
		CHECK(container->replace_object(0, Ref<WorldObject>()));
		CHECK(container->get_object(0).is_null());
	}
}

TEST_CASE("[ItemContainer] 查找和获取所有对象") {
	Ref<ItemContainer> container;
	container.instantiate();
	container->initialize(10, 0);

	Ref<WorldObject> sword = create_test_object("sword");
	Ref<WorldObject> shield = create_test_object("shield");
	Ref<WorldObject> potion = create_test_object("potion");

	container->add_object_at(1, sword);
	container->add_object_at(5, shield);
	container->add_object_at(8, potion);

	SUBCASE("查找对象槽位") {
		CHECK(container->find_object("sword") == 1);
		CHECK(container->find_object("shield") == 5);
		CHECK(container->find_object("potion") == 8);
		CHECK(container->find_object("nonexistent") == -1);
	}

	SUBCASE("获取所有对象") {
		TypedArray<WorldObject> objects = container->get_all_objects();
		CHECK(objects.size() == 3);
		CHECK(objects[0] == sword);
		CHECK(objects[1] == shield);
		CHECK(objects[2] == potion);
	}
}

TEST_CASE("[ItemContainer] 清空容器") {
	Ref<ItemContainer> container;
	container.instantiate();
	container->initialize(5, 0);

	for (int i = 0; i < 3; i++) {
		container->add_object(create_test_object("item" + itos(i)));
	}

	CHECK(container->get_used_slots() == 3);

	container->clear();

	CHECK(container->is_empty());
	CHECK(container->get_used_slots() == 0);
	for (int i = 0; i < container->get_capacity(); i++) {
		CHECK(container->get_object(i).is_null());
	}
}

// ============ 嵌套和循环引用测试 ============

TEST_CASE("[ItemContainer] 嵌套深度检查") {
	Ref<WorldObject> box1 = create_container_object("box1", 5);
	Ref<WorldObject> box2 = create_container_object("box2", 5);
	Ref<WorldObject> box3 = create_container_object("box3", 5);

	// 设置嵌套深度
	box1->get_container()->set_nesting_depth(0);
	box2->get_container()->set_nesting_depth(1);
	box3->get_container()->set_nesting_depth(2);

	SUBCASE("允许的嵌套深度") {
		CHECK(box1->container_add_object(box2));
		CHECK(box2->container_add_object(box3));
	}

	SUBCASE("超过最大嵌套深度") {
		Ref<WorldObject> box4 = create_container_object("box4", 5);
		box4->get_container()->set_nesting_depth(3);

		box1->container_add_object(box2);
		box2->container_add_object(box3);

		ERR_PRINT_OFF;
		CHECK_FALSE(box3->container_add_object(box4));
		ERR_PRINT_ON;
	}
}

TEST_CASE("[ItemContainer] 循环引用检测") {
	Ref<WorldObject> box1 = create_container_object("box1", 5);
	Ref<WorldObject> box2 = create_container_object("box2", 5);

	box1->get_container()->set_owner(box1.ptr());
	box2->get_container()->set_owner(box2.ptr());

	SUBCASE("检测自我引用") {
		ERR_PRINT_OFF;
		CHECK_FALSE(box1->container_add_object(box1));
		ERR_PRINT_ON;
	}

	SUBCASE("正常添加不同对象") {
		CHECK(box1->container_add_object(box2));
	}
}

// ============ WorldObject容器包装测试 ============

TEST_CASE("[WorldObject] 容器操作包装") {
	Ref<WorldObject> chest = create_container_object("chest", 10);
	Ref<WorldObject> sword = create_test_object("sword");
	Ref<WorldObject> shield = create_test_object("shield");

	SUBCASE("添加对象") {
		CHECK(chest->container_add_object(sword));
		CHECK(chest->get_container_used_slots() == 1);
	}

	SUBCASE("交换对象") {
		chest->container_add_object_at(0, sword);
		chest->container_add_object_at(1, shield);

		CHECK(chest->container_swap_objects(0, 1));
		CHECK(chest->container_get_object(0) == shield);
		CHECK(chest->container_get_object(1) == sword);
	}

	SUBCASE("移动对象") {
		chest->container_add_object_at(0, sword);

		CHECK(chest->container_move_object(0, 5));
		CHECK(chest->container_get_object(0).is_null());
		CHECK(chest->container_get_object(5) == sword);
	}

	SUBCASE("强制替换") {
		chest->container_add_object_at(0, sword);
		CHECK(chest->container_replace_object(0, shield));
		CHECK(chest->container_get_object(0) == shield);
	}
}

// ============ Item兼容性测试 ============

TEST_CASE("[WorldObject] Item容器兼容") {
	Ref<WorldObject> backpack = create_container_object("backpack", 5);
	Ref<Item> sword_item = create_test_item("iron_sword");
	Ref<Item> potion_item = create_test_item("health_potion");

	SUBCASE("添加Item") {
		CHECK(backpack->container_add_item(sword_item));
		Ref<Item> retrieved = backpack->container_get_item(0);
		CHECK(retrieved == sword_item);
	}

	SUBCASE("设置Item") {
		CHECK(backpack->container_set_item(0, sword_item));
		CHECK(backpack->container_get_item(0) == sword_item);
	}
}

// ============ 边界条件测试 ============

TEST_CASE("[ItemContainer] 边界条件") {
	Ref<ItemContainer> container;
	container.instantiate();

	SUBCASE("负数容量应失败") {
		ERR_PRINT_OFF;
		container->initialize(-1, 0);
		ERR_PRINT_ON;
		// 应该不会崩溃
	}

	SUBCASE("访问越界槽位") {
		container->initialize(5, 0);
		ERR_PRINT_OFF;
		CHECK(container->get_object(10).is_null());
		CHECK(container->remove_object(-1).is_null());
		ERR_PRINT_ON;
	}

	SUBCASE("添加null对象应失败") {
		container->initialize(5, 0);
		ERR_PRINT_OFF;
		CHECK_FALSE(container->add_object(Ref<WorldObject>()));
		ERR_PRINT_ON;
	}
}

// ============ 压力测试 ============

TEST_CASE("[ItemContainer] 大容量压力测试") {
	Ref<ItemContainer> container;
	container.instantiate();
	container->initialize(1000, 0);

	// 填满容器
	for (int i = 0; i < 1000; i++) {
		Ref<WorldObject> obj = create_test_object("item_" + itos(i));
		CHECK(container->add_object(obj));
	}

	CHECK(container->is_full());
	CHECK(container->get_used_slots() == 1000);

	// 清空容器
	container->clear();
	CHECK(container->is_empty());
}

TEST_CASE("[ItemContainer] 批量交换操作") {
	Ref<ItemContainer> container;
	container.instantiate();
	container->initialize(100, 0);

	// 添加50个物品
	for (int i = 0; i < 50; i++) {
		container->add_object_at(i, create_test_object("item_" + itos(i)));
	}

	// 交换所有物品位置
	for (int i = 0; i < 25; i++) {
		CHECK(container->swap_objects(i, 49 - i));
	}

	// 验证交换成功
	CHECK(container->get_used_slots() == 50);
	CHECK(container->find_object("item_0") == 49);
	CHECK(container->find_object("item_49") == 0);
}

} // namespace TestItemContainer
