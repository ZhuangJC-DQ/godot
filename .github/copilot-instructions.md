# Mu Engine — Copilot 项目指引

Godot 4.6 定制引擎，C++ Module 实现高性能框架，GDScript 负责上层逻辑。目标：开放世界游戏。

**构建：** `scons dev_build=yes` | **运行：** `./bin/Mu.windows.editor.dev.x86_64.exe --path ./Project/cpp-model`

---

## 目录结构

```
modules/game_framework/       # C++ 核心模块
  chunk.h/cpp                 # 区块数据（256×256 高度图）
  world.h/cpp                 # 世界管理（区块 HashMap 缓存）
  world_manager.h/cpp         # WorldManager 节点（暴露 GDScript）
  town_generator.h/cpp        # 城镇生成（泊松圆盘采样）
  road_generator.h/cpp        # 道路生成（MST + A*）
  item/
    item.h/cpp                # Item 纯数据类（不暴露 GDScript）
    item_manager.h/cpp        # ItemManager 单例（暴露 GDScript）
  register_types.cpp / SCsub

Project/cpp-model/            # GDScript 游戏项目
  project.godot / main.tscn
  map_visualizer.gd           # @tool 地图可视化
  autoload/item_manager_singleton.gd
```

---

## 已完成模块关键接口

**地形**：Chunk 256×256，Perlin 噪声，按需生成缓存
```cpp
WorldManager::get_chunk_data(cx, cy) -> Dictionary
WorldManager::get_tile_height(cx, cy, tx, ty) -> float
WorldManager::update_all_params(seed, freq, oct, lac, gain, curve)
```

**城镇**：泊松圆盘采样，高度+平坦度适宜度评分
```cpp
WorldManager::get_chunk_towns(cx, cy) -> Array[Dictionary]
WorldManager::get_towns_in_range(cx, cy, range) -> Array[Dictionary]
```

**道路**：Kruskal MST + A*（坡度/高程/转弯惩罚），支持虚拟跨区块查询
```cpp
struct RoadSegment { Vector<Vector2i> tiles; uint64_t town_a_id, town_b_id; float total_cost; }
```

**物品/容器**：`uint64_t` ID 管理，支持嵌套容器、序列化
```cpp
ItemManager::create_item(type_id) -> uint64_t
ItemManager::destroy_item(item_id)
ItemManager::set_as_container(item_id, max_slots)
ItemManager::add_to_container(item_id, container_id, slot=-1)
ItemManager::get_item_data(item_id) -> Dictionary
ItemManager::save_to_dict() / load_from_dict(data)
```

**坐标系**：全局 tile = `chunk * 256 + tile_local`；`ChunkCoord.to_seed()` 作 HashMap key

---

## 计划中功能

| 功能 | 优先级 |
|------|--------|
| 物品模板系统（ItemTemplate + JSON） | 高 |
| 掉落表系统 | 高 |
| 背包 UI | 高 |
| 道路可视化 | 中 |
| 城镇建筑放置 | 中 |
| 存档系统 | 中 |
| NPC 系统 | 中 |
| 网络同步 / 服务器验证 | 低 |

---

## 开发规范

### C++ 命名
- 类：`PascalCase`；方法/成员：`snake_case`；参数前缀 `p_`；常量：`UPPER_SNAKE_CASE`

### 暴露给 GDScript 的类
```cpp
// 继承 Object/Node，加 GDCLASS 宏，实现 _bind_methods()，在 register_types.cpp 注册
GDREGISTER_CLASS(ClassName);
ClassDB::bind_method(D_METHOD("method", "arg"), &Class::method);
ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "prop", PROPERTY_HINT_RANGE, "0,1,0.01"), "set_prop", "get_prop");
```

### 内存 & 确定性
- 纯数据类用 `new/delete`，Manager 析构时统一释放；错误检查：`ERR_FAIL_COND` / `ERR_FAIL_COND_V`
- 随机：`RandomPCG(chunk_seed)`；哈希打破 tie：FNV-1a（`2166136261u / 16777619u`）
- 避免每帧调用 `get_virtual_height`（会重建 FastNoiseLite）

### GDScript
- 场景节点只持有 ID，数据通过 Manager 查询：`ItemManagerSingleton.item_manager.xxx()`
- `@tool` 脚本在 setter 用 `Engine.is_editor_hint()` 判断；Autoload 放 `autoload/` 并在 `project.godot` 注册

### 新增 C++ 类检查清单
1. 创建 `.h/.cpp` → `modules/game_framework/`（或子目录）
2. 若暴露 GDScript：加 `GDCLASS`、`_bind_methods()`、`GDREGISTER_CLASS`
3. 在 `SCsub` 中添加 `.cpp`
4. `scons dev_build=yes` 重新编译
