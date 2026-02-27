class_name ItemVisualTemplate extends Resource
## 物品表现层模板
## 存储图标、描述等视觉信息，由 GDScript 管理

@export var type_id: int = 0
@export var icon: Texture2D
@export var description: String = ""
