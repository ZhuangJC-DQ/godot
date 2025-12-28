/**************************************************************************/
/*  game_framework.h                                                      */
/**************************************************************************/

#pragma once

#include "core/object/ref_counted.h"
#include "scene/main/node.h"

// 示例 1: 继承自 RefCounted 的轻量级类（不需要挂载到场景树）
class GameFramework : public RefCounted {
	GDCLASS(GameFramework, RefCounted);

private:
	String game_title;
	int game_version = 1;

protected:
	static void _bind_methods();

public:
	GameFramework();
	~GameFramework();

	// 你的游戏框架方法
	void initialize_game();
	void update_game(float delta);
	void shutdown_game();

	// Getter/Setter（可以暴露到 GDScript，但你不用脚本也能用）
	void set_game_title(const String &p_title);
	String get_game_title() const;

	void set_game_version(int p_version);
	int get_game_version() const;
};

// 示例 2: 继承自 Node 的类（可以挂载到场景树）
class GameManager : public Node {
	GDCLASS(GameManager, Node);

private:
	bool is_game_running = false;
	float elapsed_time = 0.0f;

protected:
	static void _bind_methods();
	void _notification(int p_what);

public:
	GameManager();
	~GameManager();

	// Node 的生命周期方法
	void _ready();
	void _process(double p_delta);
	void _physics_process(double p_delta);

	// 你的游戏管理方法
	void start_game();
	void pause_game();
	void stop_game();

	bool is_running() const { return is_game_running; }
};
