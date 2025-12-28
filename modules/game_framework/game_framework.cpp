/**************************************************************************/
/*  game_framework.cpp                                                    */
/**************************************************************************/

#include "game_framework.h"

#include "core/config/engine.h"
#include "core/io/resource_loader.h"
#include "core/os/os.h"

// ========== GameFramework 实现 ==========

GameFramework::GameFramework() {
	game_title = "My C++ Game";
	game_version = 1;
}

GameFramework::~GameFramework() {
	// 清理资源
}

void GameFramework::_bind_methods() {
	// 绑定方法，使其可以从脚本调用（即使你不用脚本，也建议绑定以便调试）
	ClassDB::bind_method(D_METHOD("initialize_game"), &GameFramework::initialize_game);
	ClassDB::bind_method(D_METHOD("update_game", "delta"), &GameFramework::update_game);
	ClassDB::bind_method(D_METHOD("shutdown_game"), &GameFramework::shutdown_game);

	// 绑定属性
	ClassDB::bind_method(D_METHOD("set_game_title", "title"), &GameFramework::set_game_title);
	ClassDB::bind_method(D_METHOD("get_game_title"), &GameFramework::get_game_title);
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "game_title"), "set_game_title", "get_game_title");

	ClassDB::bind_method(D_METHOD("set_game_version", "version"), &GameFramework::set_game_version);
	ClassDB::bind_method(D_METHOD("get_game_version"), &GameFramework::get_game_version);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "game_version"), "set_game_version", "get_game_version");

	// 绑定信号
	ADD_SIGNAL(MethodInfo("game_started"));
	ADD_SIGNAL(MethodInfo("game_ended"));
}

void GameFramework::initialize_game() {
	print_line("GameFramework: Initializing game - " + game_title);
	// 你的游戏初始化逻辑
	emit_signal("game_started");
}

void GameFramework::update_game(float delta) {
	// 每帧更新逻辑
	// print_line("GameFramework: Update delta = " + String::num(delta));
}

void GameFramework::shutdown_game() {
	print_line("GameFramework: Shutting down game");
	// 游戏关闭逻辑
	emit_signal("game_ended");
}

void GameFramework::set_game_title(const String &p_title) {
	game_title = p_title;
}

String GameFramework::get_game_title() const {
	return game_title;
}

void GameFramework::set_game_version(int p_version) {
	game_version = p_version;
}

int GameFramework::get_game_version() const {
	return game_version;
}

// ========== GameManager 实现 ==========

GameManager::GameManager() {
	is_game_running = false;
	elapsed_time = 0.0f;
}

GameManager::~GameManager() {
}

void GameManager::_bind_methods() {
	ClassDB::bind_method(D_METHOD("start_game"), &GameManager::start_game);
	ClassDB::bind_method(D_METHOD("pause_game"), &GameManager::pause_game);
	ClassDB::bind_method(D_METHOD("stop_game"), &GameManager::stop_game);
	ClassDB::bind_method(D_METHOD("is_running"), &GameManager::is_running);

	ADD_SIGNAL(MethodInfo("game_state_changed", PropertyInfo(Variant::BOOL, "running")));
}

void GameManager::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_READY: {
			_ready();
		} break;
		case NOTIFICATION_PROCESS: {
			_process(get_process_delta_time());
		} break;
		case NOTIFICATION_PHYSICS_PROCESS: {
			_physics_process(get_physics_process_delta_time());
		} break;
		case NOTIFICATION_EXIT_TREE: {
			// 节点从场景树移除时
			stop_game();
		} break;
	}
}

void GameManager::_ready() {
	print_line("GameManager: Ready!");
	// 节点添加到场景树时调用
}

void GameManager::_process(double p_delta) {
	if (!is_game_running) {
		return;
	}

	elapsed_time += p_delta;
	// 每帧逻辑
}

void GameManager::_physics_process(double p_delta) {
	if (!is_game_running) {
		return;
	}

	// 物理帧逻辑（固定帧率）
}

void GameManager::start_game() {
	print_line("GameManager: Starting game");
	is_game_running = true;
	elapsed_time = 0.0f;
	emit_signal("game_state_changed", true);
}

void GameManager::pause_game() {
	print_line("GameManager: Pausing game");
	is_game_running = false;
	emit_signal("game_state_changed", false);
}

void GameManager::stop_game() {
	print_line("GameManager: Stopping game");
	is_game_running = false;
	elapsed_time = 0.0f;
	emit_signal("game_state_changed", false);
}
