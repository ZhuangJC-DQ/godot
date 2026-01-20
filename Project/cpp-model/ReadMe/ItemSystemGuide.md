# 物品系统使用说明

## 概述

物品系统使用 C++ 实现核心数据管理，GDScript 处理游戏逻辑。

## 核心类

### ItemManager (C++)
- 单例模式，统一管理所有物品
- 所有物品数据存储在 C++ 层
- 通过 ID 引用物品

### Item (C++)
- 纯数据类，不暴露给 GDScript
- 支持容器功能（内部存指针）
- 避免循环引用问题

## GDScript 层

### ItemManagerSingleton (自动加载)
全局单例，封装 ItemManager 的便捷接口。

```gdscript
# 创建物品
var item_id = ItemManagerSingleton.create_item(type_id)

# 设置属性
ItemManagerSingleton.item_manager.set_item_name(item_id, "铁剑")
ItemManagerSingleton.item_manager.set_stack_count(item_id, 10)

# 容器操作
ItemManagerSingleton.item_manager.set_as_container(container_id, 20)
ItemManagerSingleton.add_to_container(item_id, container_id)

# 查询
var items = ItemManagerSingleton.get_container_items(container_id)
```

## 场景物品示例

### Chest 类
展示场景对象如何使用物品系统：

1. **初始化**: 创建物品 ID 或从存档加载
2. **数据分离**: 场景节点只存 ID，数据在 ItemManager
3. **生命周期**: 节点销毁不影响物品数据

```gdscript
# chest.gd
extends Node3D

@export var item_id: int = 0

func _ready():
    if item_id == 0:
        # 创建新宝箱
        item_id = ItemManagerSingleton.create_item(100)
        ItemManagerSingleton.item_manager.set_as_container(item_id, 10)
    else:
        # 从存档加载
        if not ItemManagerSingleton.is_valid_item(item_id):
            push_error("Invalid item ID")
```

## 测试场景

运行 `main.tscn` 查看：
- `ItemTester`: 自动运行物品系统测试
- `Chest1/2/3`: 三个宝箱展示容器功能

查看控制台输出了解测试结果。

## 网络同步准备

当前设计已为联机做好准备：

1. **双接口支持**:
   - `create_item(type)` - 单机自动分配 ID
   - `create_item_with_id(id, type)` - 服务器指定 ID

2. **序列化支持**:
   ```gdscript
   var save_data = ItemManagerSingleton.item_manager.save_to_dict()
   # 传输给服务器或保存到文件
   ```

3. **状态同步**:
   - 服务器生成 ID，客户端使用相同 ID 创建
   - 客户端只需要 `item_id` 即可重建场景节点

## 扩展方向

1. **物品类型配置**: 使用 JSON/CSV 定义物品模板
2. **物品行为**: GDScript 继承实现具体物品类（武器、消耗品等）
3. **UI 系统**: 背包界面、物品提示等
4. **拖拽系统**: 物品在容器间移动
5. **服务器验证**: 重要操作由服务器确认
