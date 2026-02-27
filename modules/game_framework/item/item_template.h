/**************************************************************************/
/*  item_template.h                                                       */
/**************************************************************************/

#ifndef ITEM_TEMPLATE_H
#define ITEM_TEMPLATE_H

#include "core/string/ustring.h"
#include "core/templates/vector.h"

// 纯数据结构，不暴露给 GDScript
// 定义物品的模板属性（数值/逻辑层）
struct ItemTemplate {
	int type_id = 0;
	String name;
	int max_stack = 1;
	bool is_container = false;
	int max_slots = 0;
	float weight = 0.0f;
	int value = 0;
	Vector<String> tags;

	bool has_tag(const String &p_tag) const {
		for (const String &tag : tags) {
			if (tag == p_tag) {
				return true;
			}
		}
		return false;
	}

	String to_string() const {
		return String("ItemTemplate{type_id=") + String::num_int64(type_id) +
				", name=" + name +
				", max_stack=" + String::num_int64(max_stack) +
				", container=" + (is_container ? "true" : "false") +
				", weight=" + String::num(weight, 1) +
				", value=" + String::num_int64(value) + "}";
	}
};

#endif // ITEM_TEMPLATE_H
