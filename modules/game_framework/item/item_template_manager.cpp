/**************************************************************************/
/*  item_template_manager.cpp                                             */
/**************************************************************************/

#include "item_template_manager.h"

#include "core/io/file_access.h"
#include "core/io/json.h"
#include "core/object/class_db.h"
#include "core/string/print_string.h"
#include "core/variant/typed_array.h"

ItemTemplateManager *ItemTemplateManager::singleton = nullptr;

ItemTemplateManager *ItemTemplateManager::get_singleton() {
	return singleton;
}

ItemTemplateManager::ItemTemplateManager() {
	ERR_FAIL_COND_MSG(singleton != nullptr, "ItemTemplateManager singleton already exists!");
	singleton = this;
}

ItemTemplateManager::~ItemTemplateManager() {
	clear_all_templates();
	singleton = nullptr;
}

void ItemTemplateManager::_bind_methods() {
	// === 加载 ===
	ClassDB::bind_method(D_METHOD("load_templates_from_json", "path"), &ItemTemplateManager::load_templates_from_json);
	ClassDB::bind_method(D_METHOD("register_template", "type_id", "data"), &ItemTemplateManager::register_template);

	// === 查询 ===
	ClassDB::bind_method(D_METHOD("has_template", "type_id"), &ItemTemplateManager::has_template);
	ClassDB::bind_method(D_METHOD("get_template_data", "type_id"), &ItemTemplateManager::get_template_data);
	ClassDB::bind_method(D_METHOD("get_all_type_ids"), &ItemTemplateManager::get_all_type_ids);
	ClassDB::bind_method(D_METHOD("get_template_count"), &ItemTemplateManager::get_template_count);
	ClassDB::bind_method(D_METHOD("get_type_ids_by_tag", "tag"), &ItemTemplateManager::get_type_ids_by_tag);

	// === 调试 ===
	ClassDB::bind_method(D_METHOD("print_all_templates"), &ItemTemplateManager::print_all_templates);
	ClassDB::bind_method(D_METHOD("clear_all_templates"), &ItemTemplateManager::clear_all_templates);
}

// ============================================
// === 加载模板 ===
// ============================================

bool ItemTemplateManager::load_templates_from_json(const String &p_path) {
	print_line(vformat("[ItemTemplateManager] Loading templates from: %s", p_path));

	Error err;
	String json_text = FileAccess::get_file_as_string(p_path, &err);
	if (err != OK) {
		ERR_PRINT(vformat("[ItemTemplateManager] Failed to open file: %s (error: %d)", p_path, err));
		return false;
	}

	Variant parsed = JSON::parse_string(json_text);
	if (parsed.get_type() != Variant::ARRAY) {
		ERR_PRINT(vformat("[ItemTemplateManager] JSON root must be an Array, got: %s", Variant::get_type_name(parsed.get_type())));
		return false;
	}

	Array templates_array = parsed;
	int loaded_count = 0;

	for (int i = 0; i < templates_array.size(); i++) {
		if (templates_array[i].get_type() != Variant::DICTIONARY) {
			WARN_PRINT(vformat("[ItemTemplateManager] Skipping non-Dictionary entry at index %d", i));
			continue;
		}

		Dictionary data = templates_array[i];

		if (!data.has("type_id")) {
			WARN_PRINT(vformat("[ItemTemplateManager] Skipping entry at index %d: missing 'type_id'", i));
			continue;
		}

		int type_id = data["type_id"];

		ItemTemplate tmpl;
		tmpl.type_id = type_id;
		tmpl.name = data.get("name", "Unknown");
		tmpl.max_stack = data.get("max_stack", 1);
		tmpl.is_container = data.get("is_container", false);
		tmpl.max_slots = data.get("max_slots", 0);
		tmpl.weight = data.get("weight", 0.0f);
		tmpl.value = data.get("value", 0);

		// 解析 tags 数组
		if (data.has("tags") && data["tags"].get_type() == Variant::ARRAY) {
			Array tags_array = data["tags"];
			for (int j = 0; j < tags_array.size(); j++) {
				tmpl.tags.push_back(tags_array[j]);
			}
		}

		templates[type_id] = tmpl;
		loaded_count++;

		print_line(vformat("[ItemTemplateManager]   Loaded: %s", tmpl.to_string()));
	}

	print_line(vformat("[ItemTemplateManager] Successfully loaded %d templates from %s", loaded_count, p_path));
	return true;
}

void ItemTemplateManager::register_template(int p_type_id, const Dictionary &p_data) {
	ItemTemplate tmpl;
	tmpl.type_id = p_type_id;
	tmpl.name = p_data.get("name", "Unknown");
	tmpl.max_stack = p_data.get("max_stack", 1);
	tmpl.is_container = p_data.get("is_container", false);
	tmpl.max_slots = p_data.get("max_slots", 0);
	tmpl.weight = p_data.get("weight", 0.0f);
	tmpl.value = p_data.get("value", 0);

	if (p_data.has("tags") && p_data["tags"].get_type() == Variant::ARRAY) {
		Array tags_array = p_data["tags"];
		for (int i = 0; i < tags_array.size(); i++) {
			tmpl.tags.push_back(tags_array[i]);
		}
	}

	templates[p_type_id] = tmpl;
	print_line(vformat("[ItemTemplateManager] Registered: %s", tmpl.to_string()));
}

// ============================================
// === 查询模板 ===
// ============================================

bool ItemTemplateManager::has_template(int p_type_id) const {
	return templates.has(p_type_id);
}

Dictionary ItemTemplateManager::get_template_data(int p_type_id) const {
	Dictionary data;
	if (!templates.has(p_type_id)) {
		ERR_PRINT(vformat("[ItemTemplateManager] Template not found: type_id=%d", p_type_id));
		return data;
	}

	const ItemTemplate &tmpl = templates[p_type_id];
	data["type_id"] = tmpl.type_id;
	data["name"] = tmpl.name;
	data["max_stack"] = tmpl.max_stack;
	data["is_container"] = tmpl.is_container;
	data["max_slots"] = tmpl.max_slots;
	data["weight"] = tmpl.weight;
	data["value"] = tmpl.value;

	Array tags_array;
	for (const String &tag : tmpl.tags) {
		tags_array.push_back(tag);
	}
	data["tags"] = tags_array;

	return data;
}

TypedArray<int> ItemTemplateManager::get_all_type_ids() const {
	TypedArray<int> result;
	for (const KeyValue<int, ItemTemplate> &kv : templates) {
		result.push_back(kv.key);
	}
	return result;
}

int ItemTemplateManager::get_template_count() const {
	return templates.size();
}

TypedArray<int> ItemTemplateManager::get_type_ids_by_tag(const String &p_tag) const {
	TypedArray<int> result;
	for (const KeyValue<int, ItemTemplate> &kv : templates) {
		if (kv.value.has_tag(p_tag)) {
			result.push_back(kv.key);
		}
	}
	return result;
}

const ItemTemplate *ItemTemplateManager::get_template(int p_type_id) const {
	if (!templates.has(p_type_id)) {
		return nullptr;
	}
	return &templates[p_type_id];
}

// ============================================
// === 调试 ===
// ============================================

void ItemTemplateManager::print_all_templates() const {
	print_line(vformat("=== ItemTemplateManager: %d templates ===", templates.size()));
	for (const KeyValue<int, ItemTemplate> &kv : templates) {
		print_line(vformat("  %s", kv.value.to_string()));
	}
}

void ItemTemplateManager::clear_all_templates() {
	templates.clear();
}
