/**************************************************************************/
/*  item.cpp                                                              */
/**************************************************************************/

#include "item.h"

#include "core/string/print_string.h"
#include "core/variant/variant.h"

Item::Item() {
}

Item::Item(uint64_t p_id, int p_type_id) :
		id(p_id), type_id(p_type_id) {
}

Item::~Item() {
	// 注意：不删除 contained_items 中的指针，由 ItemManager 统一管理
	contained_items.clear();
}

bool Item::add_item(Item *p_item, int slot) {
	ERR_FAIL_COND_V(p_item == nullptr, false);
	ERR_FAIL_COND_V(!is_container(), false);

	// 检查是否会形成循环引用
	Item *current = this;
	while (current != nullptr && current->container_id != 0) {
		if (current->id == p_item->id) {
			ERR_PRINT("Cannot add item to container: would create circular reference");
			return false;
		}
		// 这里需要通过 ItemManager 来获取父容器，暂时简单处理
		break;
	}

	if (slot == -1) {
		// 自动寻找空位
		if (contained_items.size() >= max_slots) {
			return false; // 容器已满
		}
		contained_items.push_back(p_item);
		p_item->set_container_id(id);
		return true;
	} else {
		// 指定槽位
		ERR_FAIL_COND_V(slot < 0 || slot >= max_slots, false);
		
		// 确保容器大小足够
		while (contained_items.size() <= slot) {
			contained_items.push_back(nullptr);
		}

		if (contained_items[slot] != nullptr) {
			return false; // 槽位已被占用
		}

		contained_items.write[slot] = p_item;
		p_item->set_container_id(id);
		return true;
	}
}

bool Item::remove_item(Item *p_item) {
	ERR_FAIL_COND_V(p_item == nullptr, false);
	ERR_FAIL_COND_V(!is_container(), false);

	int idx = contained_items.find(p_item);
	if (idx != -1) {
		contained_items.write[idx] = nullptr;
		p_item->set_container_id(0);
		return true;
	}
	return false;
}

Item *Item::get_item_at_slot(int slot) const {
	ERR_FAIL_COND_V(!is_container(), nullptr);
	ERR_FAIL_COND_V(slot < 0 || slot >= contained_items.size(), nullptr);
	return contained_items[slot];
}

int Item::find_item_slot(Item *p_item) const {
	ERR_FAIL_COND_V(p_item == nullptr, -1);
	ERR_FAIL_COND_V(!is_container(), -1);
	return contained_items.find(p_item);
}

bool Item::reorder_item(Item *p_item, int new_index) {
	ERR_FAIL_COND_V(p_item == nullptr, false);
	ERR_FAIL_COND_V(!is_container(), false);

	int old_idx = contained_items.find(p_item);
	ERR_FAIL_COND_V(old_idx == -1, false);

	// 1. 移除
	contained_items.remove_at(old_idx);

	// 2. 压缩空槽（移除 nullptr）
	for (int i = contained_items.size() - 1; i >= 0; i--) {
		if (contained_items[i] == nullptr) {
			contained_items.remove_at(i);
		}
	}

	// 3. 插入到目标位置（clamp 到有效范围）
	int clamped = CLAMP(new_index, 0, contained_items.size());
	contained_items.insert(clamped, p_item);

	return true;
}

void Item::clear_container() {
	for (Item *item : contained_items) {
		if (item != nullptr) {
			item->set_container_id(0);
		}
	}
	contained_items.clear();
}

String Item::to_string() const {
	String result = vformat("Item[%d] type=%d name='%s' stack=%d/%d", 
			id, type_id, name, stack_count, max_stack);
	
	if (is_container()) {
		result += vformat(" container[%d/%d items]", get_item_count(), max_slots);
	}
	
	if (container_id > 0) {
		result += vformat(" in_container=%d", container_id);
	}
	
	return result;
}
