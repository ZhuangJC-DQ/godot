/**************************************************************************/
/*  item_template_manager.h                                               */
/**************************************************************************/

#ifndef ITEM_TEMPLATE_MANAGER_H
#define ITEM_TEMPLATE_MANAGER_H

#include "item_template.h"

#include "core/object/object.h"
#include "core/templates/hash_map.h"

class ItemTemplateManager : public Object {
	GDCLASS(ItemTemplateManager, Object);

private:
	static ItemTemplateManager *singleton;

	HashMap<int, ItemTemplate> templates;

protected:
	static void _bind_methods();

public:
	static ItemTemplateManager *get_singleton();

	ItemTemplateManager();
	~ItemTemplateManager();

	// === 加载模板 ===

	// 从 JSON 文件加载模板数据（res:// 路径）
	bool load_templates_from_json(const String &p_path);

	// 手动注册一个模板（GDScript 也可调用）
	void register_template(int p_type_id, const Dictionary &p_data);

	// === 查询模板 ===

	// 检查模板是否存在
	bool has_template(int p_type_id) const;

	// 获取模板数据（Dictionary，暴露给 GDScript）
	Dictionary get_template_data(int p_type_id) const;

	// 获取所有模板 type_id 列表
	TypedArray<int> get_all_type_ids() const;

	// 获取模板数量
	int get_template_count() const;

	// 按标签查询模板
	TypedArray<int> get_type_ids_by_tag(const String &p_tag) const;

	// === C++ 内部接口（不暴露给 GDScript）===

	// 获取模板指针，用于 ItemManager 内部
	const ItemTemplate *get_template(int p_type_id) const;

	// === 调试 ===
	void print_all_templates() const;
	void clear_all_templates();
};

#endif // ITEM_TEMPLATE_MANAGER_H
