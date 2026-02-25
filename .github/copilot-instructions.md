# Copilot 项目指引 — Mu Engine 游戏框架

> 最后更新：2026-02-25

---

## 一、项目概览

本项目是一个**基于 Godot 4.6 源码定制的游戏引擎（代号 Mu）**，通过 C++ Module 方式在引擎层实现高性能游戏框架，GDScript 负责上层逻辑和可视化。  
目标是制作一款**开放世界类游戏**，具备程序化地形生成、城镇分布、道路网络、物品/容器系统等核心系统。

### 技术栈

| 层级 | 技术 | 说明 |
|------|------|------|
| 引擎层 | C++ (Godot Module) | `modules/game_framework/` — 高性能核心系统 |
| 脚本层 | GDScript | `Project/cpp-model/` — 游戏逻辑、UI、可视化 |
| 构建 | SCons + Python | `scons dev_build=yes` 编译引擎 |
| 物理 | Jolt Physics | 3D 物理引擎 |
| 渲染 | Forward Plus + D3D12 | Windows 平台 |

### 目录结构

```
godot/                          # Godot 引擎源码（已定制）
├── modules/game_framework/     # 【核心】C++ 游戏框架模块
│   ├── chunk.h/cpp             # 区块数据（256×256 高度图）
│   ├── world.h/cpp             # 世界管理（区块 HashMap 缓存）
│   ├── world_manager.h/cpp     # WorldManager 节点（暴露给 GDScript）
│   ├── town_generator.h/cpp    # 城镇生成器（泊松圆盘采样）
│   ├── road_generator.h/cpp    # 道路生成器（MST + A*）
│   ├── item/
│   │   ├── item.h/cpp          # Item 纯数据类（不暴露给 GDScript）
│   │   └── item_manager.h/cpp  # ItemManager 单例（暴露给 GDScript）
│   ├── register_types.cpp      # 模块类型注册
│   ├── SCsub                   # 构建脚本
│   └── config.py               # 模块配置
├── Project/
│   ├── cpp-model/              # 【游戏项目】GDScript 主项目
│   │   ├── project.godot       # 项目配置
│   │   ├── main.tscn           # 主场景
│   │   ├── map_visualizer.gd   # 地图可视化(@tool 脚本)
│   │   ├── chest.gd/tscn       # 宝箱示例
│   │   ├── test_items.gd       # 物品系统测试
│   │   └── autoload/
│   │       └── item_manager_singleton.gd  # ItemManager 全局单例
│   └── Resource/               # 美术资源（Kenney 素材包）
└── bin/                        # 编译输出
    └── Mu.windows.editor.dev.x86_64.exe
```

---

## 二、已完成的功能模块

### ✅ 1. 程序化地形生成系统

**状态：已完成，可用**

- **Chunk 系统** — 256×256 瓦片，Perlin 噪声高度图，按需生成并缓存
- **NoiseConfig** — 可调参数：frequency、octaves、lacunarity、gain、terrain_curve
- **高度曲线** — 低地变平（平原），高地保持起伏（山地）
- **WorldManager 节点** — 继承 Node，Inspector 可编辑参数，绑定到 GDScript
- **MapVisualizer (@tool)** — 编辑器内实时预览地形，支持多 Chunk 网格、降采样、顶点色

关键接口：
```cpp
// C++ 层
WorldManager::get_chunk_data(chunk_x, chunk_y) -> Dictionary
WorldManager::get_tile_height(chunk_x, chunk_y, tile_x, tile_y) -> float
WorldManager::update_all_params(seed, freq, oct, lac, gain, curve) // 批量更新
```

### ✅ 2. 城镇生成系统

**状态：已完成，可用**

- **TownGenerator** — 泊松圆盘（Dart-throwing）采样，确定性生成
- **TownConfig** — 可调参数：min/max_height、max_variance、min_distance_tiles、max_towns_per_chunk
- **适宜度评分** — 综合高度适宜度 + 平坦度评分
- **可视化** — 城镇标记球体，颜色按适宜度从红到绿渐变

关键接口：
```cpp
WorldManager::get_town_count(chunk_x, chunk_y) -> int
WorldManager::get_chunk_towns(chunk_x, chunk_y) -> Array[Dictionary]
WorldManager::get_towns_in_range(center_x, center_y, range) -> Array[Dictionary]
```

### ✅ 3. 道路网络生成系统

**状态：已完成，可用**

- **虚拟城镇查询** — 无需实际生成 Chunk 即可查询城镇位置（复现生成算法）
- **最小生成树（MST）** — Kruskal 算法，确定性排序，全局唯一 TownGlobalId
- **A\* 寻路** — 考虑坡度惩罚、高程惩罚、转弯惩罚，确定性哈希打破 tie
- **路径裁剪** — 全局路径裁剪到 Chunk 本地坐标
- **RoadSegment** — 存储在 Chunk 内，包含 tile 序列 + 起/终城镇 ID + 总成本

关键结构：
```cpp
struct RoadConfig { max_slope, slope_penalty_factor, altitude_penalty_threshold, turn_penalty, search_radius_chunks, astar_max_iterations }
struct RoadSegment { Vector<Vector2i> tiles, town_a_id, town_b_id, total_cost }
```

### ✅ 4. 物品与容器系统

**状态：已完成，可用**

- **Item** — C++ 纯数据类（id, type_id, name, stack_count, max_stack, container_id, contained_items）
- **ItemManager 单例** — HashMap<uint64_t, Item*> 统一管理所有物品生命周期
- **容器功能** — 物品可设为容器（max_slots），支持嵌套容器、自动寻位、槽位操作
- **序列化** — save_to_dict / load_from_dict，两遍加载（先创建物品，再恢复容器关系）
- **联机预留** — create_item (单机自动ID) / create_item_with_id (服务器指定ID)
- **GDScript 单例** — ItemManagerSingleton Autoload，便捷接口封装

关键接口：
```cpp
ItemManager::create_item(type_id) -> uint64_t
ItemManager::destroy_item(item_id)
ItemManager::set_as_container(item_id, max_slots)
ItemManager::add_to_container(item_id, container_id, slot)
ItemManager::get_item_data(item_id) -> Dictionary
ItemManager::save_to_dict() / load_from_dict(data)
```

### ✅ 5. 示例场景与测试

**状态：已完成**

- **Chest 场景** — 宝箱类，展示物品系统在场景节点中的使用模式
- **test_items.gd** — 自动化测试：基础创建、容器、嵌套容器、序列化
- **main.tscn** — 集成场景，包含 WorldManager + MapVisualizer + 地形网格

---

## 三、尚未完成 / 计划中的功能

| 功能 | 状态 | 优先级 | 说明 |
|------|------|--------|------|
| 物品类型模板系统 | ⬜ 未开始 | 高 | JSON/CSV 定义物品模板（ItemTemplate），运行时从模板创建实例 |
| 掉落表系统 | ⬜ 未开始 | 高 | loot_tables.json，按概率、权重生成掉落 |
| 背包 UI | ⬜ 未开始 | 高 | 背包界面、物品拖拽、物品提示 |
| 玩家角色 | ⬜ 未开始 | 高 | CharacterBody3D，移动、相机控制 |
| 道路可视化 | ⬜ 未开始 | 中 | 在地形网格上渲染道路，标记路径 tile |
| 城镇建筑放置 | ⬜ 未开始 | 中 | 在城镇位置生成建筑模型（Kenney 素材） |
| 存档系统 | ⬜ 未开始 | 中 | 文件 I/O，自动保存/加载 |
| NPC 系统 | ⬜ 未开始 | 中 | NPC 生成、对话、交易逻辑 |
| 网络同步 | ⬜ 未开始 | 低 | 多人联机，服务器/客户端架构 |
| 服务器验证 | ⬜ 未开始 | 低 | 关键操作服务器确认 |
| 音效系统 | ⬜ 未开始 | 低 | 环境音、交互音效 |

---

## 四、开发规范

### 4.1 C++ 模块开发规范

#### 文件组织
- 所有 C++ 游戏框架代码放在 `modules/game_framework/` 下
- 子系统用子文件夹组织（如 `item/`）
- 每个类一对 `.h/.cpp` 文件
- 新文件必须在 `SCsub` 中注册

#### 命名约定
- 类名：`PascalCase`（如 `WorldManager`、`ItemManager`）
- 方法名：`snake_case`（如 `get_chunk_data`、`create_item`）
- 成员变量：`snake_case`（如 `next_id`、`max_slots`）
- 方法参数前缀 `p_`（如 `p_seed`、`p_config`）
- 常量/宏：`UPPER_SNAKE_CASE`（如 `CHUNK_SIZE`、`GDCLASS`）
- 头文件保护：`#pragma once` 或 `#ifndef XXX_H / #define XXX_H`

#### 类型系统分层
```
暴露给 GDScript 的类:
  - 继承 Object 或 Node，使用 GDCLASS 宏
  - 在 _bind_methods() 中注册方法和属性
  - 在 register_types.cpp 中 GDREGISTER_CLASS

纯 C++ 内部数据类:
  - 不使用 GDCLASS，不暴露给 GDScript
  - 由 Manager 类管理生命周期
  - 示例：Item, Chunk, TownInfo, RoadSegment
```

#### GDScript 绑定模式
```cpp
// 方法绑定
ClassDB::bind_method(D_METHOD("method_name", "arg1", "arg2"), &ClassName::method_name);
// 带默认值
ClassDB::bind_method(D_METHOD("add_to_container", "item_id", "container_id", "slot"),
    &ItemManager::add_to_container, DEFVAL(-1));
// 属性注册
ADD_GROUP("Group Name", "");
ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "prop_name", PROPERTY_HINT_RANGE, "0.0,1.0,0.01"),
    "set_prop_name", "get_prop_name");
```

#### 内存管理
- 纯数据对象（如 Item）使用 `new/delete`，由 Manager 统一管理
- Manager 的析构函数必须清理所有持有的对象
- Chunk 使用 HashMap 缓存，World 析构时统一释放
- 错误检查使用 `ERR_FAIL_COND` / `ERR_FAIL_COND_V` 宏

#### 确定性要求
- 所有程序化生成算法必须支持 seed 确定性
- 使用 `RandomPCG` 作为确定性随机数生成器
- 排序比较器在值相等时使用坐标/ID 哈希打破 tie
- 使用 FNV-1a 哈希（`2166136261u` / `16777619u`）

#### 性能注意
- 使用 `OS::get_singleton()->get_ticks_usec()` 计时关键路径
- 避免每帧高频调用 `get_virtual_height`（它会重新创建 FastNoiseLite）
- Chunk 生成后缓存在 HashMap 中，避免重复生成
- 批量参数更新使用 `update_all_params()` 只清除一次缓存

### 4.2 GDScript 开发规范

#### 命名约定
- 类名：`PascalCase`（`class_name Chest`）
- 函数/变量：`snake_case`
- 常量：`UPPER_SNAKE_CASE`（`const CHUNK_SIZE = 256`）
- 私有方法前缀 `_`（`_initialize_loot()`）
- 信号名：`snake_case`

#### 自动加载（Autoload）
- 全局单例放在 `autoload/` 目录
- 在 `project.godot` 的 `[autoload]` 节注册
- 当前已有：`ItemManagerSingleton`
- 访问 C++ Manager：`ItemManagerSingleton.item_manager.xxx()`

#### @tool 脚本
- 编辑器可视化脚本使用 `@tool` 注解
- 在 setter 中判断 `Engine.is_editor_hint()` 再触发更新
- 避免在 `_ready()` 中执行重量级操作（区分编辑器/运行时）

#### 场景-数据分离模式
```gdscript
# 场景节点只持有 ID，所有数据查询通过 Manager
@export var item_id: int = 0

func _ready():
    if item_id == 0:
        item_id = ItemManagerSingleton.create_item(type_id)  # 新建
    else:
        assert(ItemManagerSingleton.is_valid_item(item_id))  # 加载验证
```

### 4.3 构建与运行

```powershell
# 编译引擎（开发模式）
scons dev_build=yes

# 运行项目
./bin/Mu.windows.editor.dev.x86_64.exe --path ./Project/cpp-model

# VS Code 任务
# build: scons dev_build=yes
# run:   执行编译后的引擎
```

### 4.4 新增 C++ 类检查清单

1. 创建 `.h/.cpp` 文件，放入 `modules/game_framework/` 或其子目录
2. 如需暴露给 GDScript：
   - 继承 `Object` 或 `Node`
   - 添加 `GDCLASS(ClassName, ParentClass)` 宏
   - 实现 `static void _bind_methods()`
   - 在 `register_types.cpp` 中 `GDREGISTER_CLASS(ClassName)`
3. 在 `SCsub` 中添加 `.cpp` 文件编译
4. 重新编译引擎：`scons dev_build=yes`

---

## 五、架构设计决策

### 5.1 为什么用 C++ Module 而非 GDExtension

- 直接访问引擎内部 API（FastNoiseLite、RandomPCG 等）
- 无 FFI 开销，对高频操作（地形生成、A\*）性能关键
- 与引擎同步编译，无版本兼容问题

### 5.2 为什么 Item 不暴露给 GDScript

- GDScript 通过 `uint64_t` ID 引用物品，避免 GC 与 C++ 指针交叉管理
- 所有操作通过 `ItemManager` 单例中转，保证数据一致性
- 便于未来网络同步（只需传递 ID）

### 5.3 世界生成的确定性策略

- 相同 seed → 相同地形 → 相同城镇 → 相同道路
- 虚拟查询（`query_virtual_towns_in_chunk`）不实际生成 Chunk，用于道路规划跨区域查询
- 所有随机操作使用 `RandomPCG(chunk_seed)` 或 FNV 哈希，不依赖全局状态

### 5.4 Chunk 坐标系

```
Chunk 坐标     → (chunk_x, chunk_y)，整数，可为负
Tile 坐标      → (tile_x, tile_y)，[0, CHUNK_SIZE-1]，Chunk 内局部
全局 Tile 坐标 → chunk_x * CHUNK_SIZE + tile_x
ChunkCoord.to_seed() → 用于 HashMap 的 key
```

---

## 六、资源说明

### 美术资源（Project/Resource/）

| 素材包 | 内容 | 许可 |
|--------|------|------|
| kenney_city-kit-commercial | 商业城市建筑模型 | CC0 |
| kenney_city-kit-roads | 道路模型 | CC0 |
| kenney_food-kit | 食物模型 | CC0 |
| kenney_mini-market | 迷你超市模型 | CC0 |
| kenney_prototype-kit | 原型开发模型 | CC0 |
| kenney_ui-pack-space-expansion | UI 素材 | CC0 |

---

## 七、常见问题

### Q: 如何添加新的物品类型？
目前物品类型通过 `type_id`（int）区分，尚无模板系统。  
临时方案：在 GDScript 中创建物品后手动设置属性。  
计划方案：实现 `ItemTemplate` JSON 配置 + `ItemFactory`。

### Q: 如何修改地形生成参数？
方法一：在编辑器 Inspector 中修改 `WorldManager` 节点属性。  
方法二：在 `MapVisualizer` 的导出参数中修改并点击 `apply_terrain_params`。  
方法三：GDScript 代码中调用 `world_manager.update_all_params()`。

### Q: 编译后场景不更新？
确保在 `MapVisualizer` 中勾选 `regenerate` 或点击 `apply_terrain_params` 触发重新生成。  
注意：修改 C++ 代码后必须重新编译引擎（`scons dev_build=yes`）。
