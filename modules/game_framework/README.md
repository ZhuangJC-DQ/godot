# Game Framework 模块

这是一个自定义的 C++ 游戏框架模块，演示如何在 Godot 源码中使用纯 C++ 开发游戏。

## 使用方法

### 1. 编译引擎
```bash
scons dev_build=yes
```

模块会自动检测并编译。

### 2. 禁用特定模块（可选）
如果不需要某些内置模块，可以在编译时禁用：
```bash
scons dev_build=yes module_gdscript_enabled=no module_mono_enabled=no
```

### 3. 在 C++ 中使用你的框架

#### 方法 A: 修改主入口（完全 C++ 游戏）
编辑 `main/main.cpp`，在游戏启动时初始化你的框架：

```cpp
#include "modules/game_framework/game_framework.h"

// 在 Main::start() 或 Main::setup() 中
Ref<GameFramework> game = memnew(GameFramework);
game->initialize_game();
```

#### 方法 B: 创建自定义节点
在场景中使用 GameManager 节点：

```cpp
// 在你的代码中创建
GameManager *manager = memnew(GameManager);
get_tree()->get_root()->add_child(manager);
manager->start_game();
```

#### 方法 C: 注册为单例
修改 `register_types.cpp`，将你的框架注册为全局单例：

```cpp
GameFramework *game_framework = memnew(GameFramework);
Engine::get_singleton()->add_singleton(Engine::Singleton("GameFramework", game_framework));
```

然后在任何地方访问：
```cpp
GameFramework *game = Object::cast_to<GameFramework>(
    Engine::get_singleton()->get_singleton_object("GameFramework")
);
```

### 4. 纯 C++ 开发模式

如果完全不需要编辑器和脚本：

1. 修改 `main/main.cpp` 添加你的游戏逻辑
2. 编译时指定不启动编辑器：
```bash
scons dev_build=yes target=template_debug
```

3. 运行可执行文件：
```bash
./bin/godot.windows.template_debug.dev.x86_64.exe
```

## 文件结构

```
game_framework/
├── config.py              # 模块配置
├── SCsub                  # 构建脚本
├── register_types.h       # 模块注册头文件
├── register_types.cpp     # 模块注册实现
├── game_framework.h       # 你的框架头文件
├── game_framework.cpp     # 你的框架实现
└── README.md             # 本文档
```

## 扩展建议

1. **添加子系统**：创建 `systems/` 目录存放各个子系统
2. **ECS 架构**：实现实体-组件-系统架构
3. **资源管理**：创建自定义资源加载器
4. **网络层**：添加网络通信模块
5. **物理扩展**：扩展物理引擎功能

## 注意事项

- 模块会在引擎编译时自动包含
- 所有类都需要正确绑定才能在编辑器中使用
- 使用 `print_line()` 进行调试输出
- 使用 Godot 的内存管理：`memnew()` 和 `memdelete()`
