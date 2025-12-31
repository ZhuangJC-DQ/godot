/**************************************************************************/
/*  register_types.cpp                                                    */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#include "register_types.h"

#include "game_framework.h"
#include "item.h"
#include "world_object.h"

#include "core/object/class_db.h"

void initialize_game_framework_module(ModuleInitializationLevel p_level) {
	// 模块初始化级别说明：
	// MODULE_INITIALIZATION_LEVEL_CORE - 核心系统初始化
	// MODULE_INITIALIZATION_LEVEL_SERVERS - 服务器初始化
	// MODULE_INITIALIZATION_LEVEL_SCENE - 场景系统初始化
	// MODULE_INITIALIZATION_LEVEL_EDITOR - 编辑器初始化

	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}

	// 注册你的类到 Godot 的类系统
	GDREGISTER_CLASS(GameFramework);
	GDREGISTER_CLASS(Item);
	GDREGISTER_CLASS(WorldObject);

	// 你可以在这里注册更多的类
	// GDREGISTER_CLASS(YourOtherClass);

	// 如果需要创建单例（全局访问的对象）
	// GameFramework *game_framework = memnew(GameFramework);
	// Engine::get_singleton()->add_singleton(Engine::Singleton("GameFramework", GameFramework::get_singleton()));
}

void uninitialize_game_framework_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}

	// 清理资源
	// 如果创建了单例，需要在这里删除
	// memdelete(GameFramework::get_singleton());
}
