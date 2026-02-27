/**************************************************************************/
/*  item_manager.cpp                                                      */
/**************************************************************************/

#include "item_manager.h"
#include "item_template_manager.h"

#include "core/object/class_db.h"
#include "core/string/print_string.h"
#include "core/variant/variant.h"

ItemManager *ItemManager::singleton = nullptr;

ItemManager *ItemManager::get_singleton() {
	return singleton;
}

ItemManager::ItemManager() {
	ERR_FAIL_COND_MSG(singleton != nullptr, "ItemManager singleton already exists!");
	singleton = this;
}

ItemManager::~ItemManager() {
	clear_all_items();
	singleton = nullptr;
}

void ItemManager::_bind_methods() {
	// === 物品生命周期 ===
	ClassDB::bind_method(D_METHOD("create_item", "type_id"), &ItemManager::create_item);
	ClassDB::bind_method(D_METHOD("create_item_with_id", "id", "type_id"), &ItemManager::create_item_with_id);
	ClassDB::bind_method(D_METHOD("destroy_item", "item_id"), &ItemManager::destroy_item);
	ClassDB::bind_method(D_METHOD("is_valid_item", "item_id"), &ItemManager::is_valid_item);
	ClassDB::bind_method(D_METHOD("get_item_count"), &ItemManager::get_item_count);

	// === 基础属性 ===
	ClassDB::bind_method(D_METHOD("get_item_name", "item_id"), &ItemManager::get_item_name);
	ClassDB::bind_method(D_METHOD("set_item_name", "item_id", "name"), &ItemManager::set_item_name);
	ClassDB::bind_method(D_METHOD("get_item_type", "item_id"), &ItemManager::get_item_type);
	ClassDB::bind_method(D_METHOD("get_stack_count", "item_id"), &ItemManager::get_stack_count);
	ClassDB::bind_method(D_METHOD("set_stack_count", "item_id", "count"), &ItemManager::set_stack_count);
	ClassDB::bind_method(D_METHOD("get_max_stack", "item_id"), &ItemManager::get_max_stack);
	ClassDB::bind_method(D_METHOD("set_max_stack", "item_id", "max"), &ItemManager::set_max_stack);

	// === 容器操作 ===
	ClassDB::bind_method(D_METHOD("set_as_container", "item_id", "max_slots"), &ItemManager::set_as_container);
	ClassDB::bind_method(D_METHOD("is_container", "item_id"), &ItemManager::is_container);
	ClassDB::bind_method(D_METHOD("get_max_slots", "item_id"), &ItemManager::get_max_slots);
	ClassDB::bind_method(D_METHOD("add_to_container", "item_id", "container_id", "slot"), &ItemManager::add_to_container, DEFVAL(-1));
	ClassDB::bind_method(D_METHOD("remove_from_container", "item_id"), &ItemManager::remove_from_container);
	ClassDB::bind_method(D_METHOD("get_container_id", "item_id"), &ItemManager::get_container_id);
	ClassDB::bind_method(D_METHOD("get_slot_in_container", "item_id"), &ItemManager::get_slot_in_container);
	ClassDB::bind_method(D_METHOD("get_item_at_slot", "container_id", "slot"), &ItemManager::get_item_at_slot);
	ClassDB::bind_method(D_METHOD("get_container_items", "container_id"), &ItemManager::get_container_items);
	ClassDB::bind_method(D_METHOD("clear_container", "container_id"), &ItemManager::clear_container);

	// === 批量操作 ===
	ClassDB::bind_method(D_METHOD("get_item_data", "item_id"), &ItemManager::get_item_data);
	ClassDB::bind_method(D_METHOD("set_item_data", "item_id", "data"), &ItemManager::set_item_data);

	// === 序列化 ===
	ClassDB::bind_method(D_METHOD("save_to_dict"), &ItemManager::save_to_dict);
	ClassDB::bind_method(D_METHOD("load_from_dict", "data"), &ItemManager::load_from_dict);

	// === 调试 ===
	ClassDB::bind_method(D_METHOD("print_item", "item_id"), &ItemManager::print_item);
	ClassDB::bind_method(D_METHOD("print_all_items"), &ItemManager::print_all_items);
	ClassDB::bind_method(D_METHOD("clear_all_items"), &ItemManager::clear_all_items);
}

// ============================================
// === 物品生命周期 ===
// ============================================

uint64_t ItemManager::create_item(int type_id) {
	uint64_t id = next_id++;
	Item *item = new Item(id, type_id);

	// 从模板自动填充属性
	ItemTemplateManager *tmpl_mgr = ItemTemplateManager::get_singleton();
	if (tmpl_mgr) {
		const ItemTemplate *tmpl = tmpl_mgr->get_template(type_id);
		if (tmpl) {
			item->set_name(tmpl->name);
			item->set_max_stack(tmpl->max_stack);
			if (tmpl->is_container) {
				item->set_max_slots(tmpl->max_slots);
			}
			print_line(vformat("[ItemManager] Created item from template: id=%d, type=%d, name=%s", id, type_id, tmpl->name));
		} else {
			print_line(vformat("[ItemManager] Created item without template: id=%d, type=%d (no template found)", id, type_id));
		}
	} else {
		print_line(vformat("[ItemManager] Created item (no template manager): id=%d, type=%d", id, type_id));
	}

	items[id] = item;
	return id;
}

uint64_t ItemManager::create_item_with_id(uint64_t id, int type_id) {
	ERR_FAIL_COND_V_MSG(items.has(id), 0, vformat("Item with ID %d already exists!", id));

	Item *item = new Item(id, type_id);
	items[id] = item;

	// 更新 next_id 以避免冲突（单机模式使用）
	if (id >= next_id) {
		next_id = id + 1;
	}

	return id;
}

void ItemManager::destroy_item(uint64_t item_id) {
	Item *item = get_item(item_id);
	ERR_FAIL_COND(!item);

	// 如果物品在容器中，先从容器移除
	if (item->get_container_id() > 0) {
		Item *container = get_item(item->get_container_id());
		if (container) {
			container->remove_item(item);
		}
	}

	// 如果物品是容器，移除所有内部物品的引用
	if (item->is_container()) {
		item->clear_container();
	}

	// 删除物品
	items.erase(item_id);
	delete item;
}

bool ItemManager::is_valid_item(uint64_t item_id) const {
	return items.has(item_id);
}

int ItemManager::get_item_count() const {
	return items.size();
}

// ============================================
// === 基础属性读写 ===
// ============================================

String ItemManager::get_item_name(uint64_t item_id) const {
	Item *item = get_item(item_id);
	ERR_FAIL_COND_V(!item, "");
	return item->get_name();
}

void ItemManager::set_item_name(uint64_t item_id, const String &name) {
	Item *item = get_item(item_id);
	ERR_FAIL_COND(!item);
	item->set_name(name);
}

int ItemManager::get_item_type(uint64_t item_id) const {
	Item *item = get_item(item_id);
	ERR_FAIL_COND_V(!item, -1);
	return item->get_type_id();
}

int ItemManager::get_stack_count(uint64_t item_id) const {
	Item *item = get_item(item_id);
	ERR_FAIL_COND_V(!item, 0);
	return item->get_stack_count();
}

void ItemManager::set_stack_count(uint64_t item_id, int count) {
	Item *item = get_item(item_id);
	ERR_FAIL_COND(!item);
	item->set_stack_count(count);
}

int ItemManager::get_max_stack(uint64_t item_id) const {
	Item *item = get_item(item_id);
	ERR_FAIL_COND_V(!item, 1);
	return item->get_max_stack();
}

void ItemManager::set_max_stack(uint64_t item_id, int max) {
	Item *item = get_item(item_id);
	ERR_FAIL_COND(!item);
	item->set_max_stack(max);
}

// ============================================
// === 容器操作 ===
// ============================================

void ItemManager::set_as_container(uint64_t item_id, int max_slots) {
	Item *item = get_item(item_id);
	ERR_FAIL_COND(!item);
	ERR_FAIL_COND(max_slots < 0);
	item->set_max_slots(max_slots);
}

bool ItemManager::is_container(uint64_t item_id) const {
	Item *item = get_item(item_id);
	ERR_FAIL_COND_V(!item, false);
	return item->is_container();
}

int ItemManager::get_max_slots(uint64_t item_id) const {
	Item *item = get_item(item_id);
	ERR_FAIL_COND_V(!item, 0);
	return item->get_max_slots();
}

bool ItemManager::add_to_container(uint64_t item_id, uint64_t container_id, int slot) {
	Item *item = get_item(item_id);
	Item *container = get_item(container_id);
	ERR_FAIL_COND_V(!item, false);
	ERR_FAIL_COND_V(!container, false);
	ERR_FAIL_COND_V(!container->is_container(), false);

	// 如果物品已经在其他容器中，先移除
	if (item->get_container_id() > 0) {
		Item *old_container = get_item(item->get_container_id());
		if (old_container) {
			old_container->remove_item(item);
		}
	}

	return container->add_item(item, slot);
}

bool ItemManager::remove_from_container(uint64_t item_id) {
	Item *item = get_item(item_id);
	ERR_FAIL_COND_V(!item, false);

	uint64_t container_id = item->get_container_id();
	if (container_id == 0) {
		return false; // 物品不在任何容器中
	}

	Item *container = get_item(container_id);
	if (!container) {
		// 容器已被删除，直接清除引用
		item->set_container_id(0);
		return true;
	}

	return container->remove_item(item);
}

uint64_t ItemManager::get_container_id(uint64_t item_id) const {
	Item *item = get_item(item_id);
	ERR_FAIL_COND_V(!item, 0);
	return item->get_container_id();
}

int ItemManager::get_slot_in_container(uint64_t item_id) const {
	Item *item = get_item(item_id);
	ERR_FAIL_COND_V(!item, -1);

	uint64_t container_id = item->get_container_id();
	if (container_id == 0) {
		return -1;
	}

	Item *container = get_item(container_id);
	ERR_FAIL_COND_V(!container, -1);

	return container->find_item_slot(item);
}

uint64_t ItemManager::get_item_at_slot(uint64_t container_id, int slot) const {
	Item *container = get_item(container_id);
	ERR_FAIL_COND_V(!container, 0);
	ERR_FAIL_COND_V(!container->is_container(), 0);

	Item *item = container->get_item_at_slot(slot);
	return item ? item->get_id() : 0;
}

TypedArray<int> ItemManager::get_container_items(uint64_t container_id) const {
	TypedArray<int> result;
	Item *container = get_item(container_id);
	ERR_FAIL_COND_V(!container, result);
	ERR_FAIL_COND_V(!container->is_container(), result);

	const Vector<Item *> &items_vec = container->get_contained_items();
	for (const Item *item : items_vec) {
		if (item != nullptr) {
			result.push_back(item->get_id());
		} else {
			result.push_back(0); // 空槽位
		}
	}

	return result;
}

void ItemManager::clear_container(uint64_t container_id) {
	Item *container = get_item(container_id);
	ERR_FAIL_COND(!container);
	ERR_FAIL_COND(!container->is_container());

	container->clear_container();
}

// ============================================
// === 批量操作 ===
// ============================================

Dictionary ItemManager::get_item_data(uint64_t item_id) const {
	Dictionary data;
	Item *item = get_item(item_id);
	ERR_FAIL_COND_V(!item, data);

	data["id"] = item->get_id();
	data["type_id"] = item->get_type_id();
	data["name"] = item->get_name();
	data["stack_count"] = item->get_stack_count();
	data["max_stack"] = item->get_max_stack();
	data["container_id"] = item->get_container_id();
	data["is_container"] = item->is_container();
	data["max_slots"] = item->get_max_slots();

	return data;
}

void ItemManager::set_item_data(uint64_t item_id, const Dictionary &data) {
	Item *item = get_item(item_id);
	ERR_FAIL_COND(!item);

	if (data.has("name")) {
		item->set_name(data["name"]);
	}
	if (data.has("stack_count")) {
		item->set_stack_count(data["stack_count"]);
	}
	if (data.has("max_stack")) {
		item->set_max_stack(data["max_stack"]);
	}
	if (data.has("max_slots")) {
		item->set_max_slots(data["max_slots"]);
	}
}

// ============================================
// === 序列化 ===
// ============================================

Dictionary ItemManager::save_to_dict() const {
	Dictionary save_data;
	save_data["next_id"] = next_id;

	Array items_array;
	for (const KeyValue<uint64_t, Item *> &kv : items) {
		Item *item = kv.value;
		Dictionary item_data = get_item_data(item->get_id());

		// 保存容器内容
		if (item->is_container()) {
			Array container_items;
			const Vector<Item *> &items_vec = item->get_contained_items();
			for (const Item *contained : items_vec) {
				container_items.push_back(contained ? (int)contained->get_id() : 0);
			}
			item_data["container_items"] = container_items;
		}

		items_array.push_back(item_data);
	}
	save_data["items"] = items_array;

	return save_data;
}

void ItemManager::load_from_dict(const Dictionary &data) {
	clear_all_items();

	if (data.has("next_id")) {
		next_id = data["next_id"];
	}

	if (!data.has("items")) {
		return;
	}

	Array items_array = data["items"];

	// 第一遍：创建所有物品
	for (int i = 0; i < items_array.size(); i++) {
		Dictionary item_data = items_array[i];
		uint64_t id = item_data["id"];
		int type_id = item_data["type_id"];

		create_item_with_id(id, type_id);
		set_item_data(id, item_data);
	}

	// 第二遍：恢复容器关系
	for (int i = 0; i < items_array.size(); i++) {
		Dictionary item_data = items_array[i];
		if (!item_data.has("container_items")) {
			continue;
		}

		uint64_t container_id = item_data["id"];
		Array container_items = item_data["container_items"];

		for (int j = 0; j < container_items.size(); j++) {
			int item_id = container_items[j];
			if (item_id > 0) {
				add_to_container(item_id, container_id, j);
			}
		}
	}
}

// ============================================
// === 调试接口 ===
// ============================================

void ItemManager::print_item(uint64_t item_id) const {
	Item *item = get_item(item_id);
	ERR_FAIL_COND(!item);
	print_line(item->to_string());
}

void ItemManager::print_all_items() const {
	print_line(vformat("=== ItemManager: %d items ===", items.size()));
	for (const KeyValue<uint64_t, Item *> &kv : items) {
		print_line(kv.value->to_string());
	}
}

void ItemManager::clear_all_items() {
	for (KeyValue<uint64_t, Item *> &kv : items) {
		delete kv.value;
	}
	items.clear();
	next_id = 1;
}

// ============================================
// === 内部辅助方法 ===
// ============================================

Item *ItemManager::get_item(uint64_t item_id) const {
	if (items.has(item_id)) {
		return items[item_id];
	}
	return nullptr;
}
