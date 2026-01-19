item_templates.json (静态属性)
         ↓ 引用
loot_tables.json (掉落规则)
         ↓ 填充
container_templates.json (容器行为)
         ↓ 实例化
         
运行时内存:
┌─────────────────────────────────┐
│ ItemTemplate (10001)            │
│ - name: "新手剑"                 │
│ - attributes: {attack: 10}      │
└─────────────────────────────────┘
         ↓ 创建实例
┌─────────────────────────────────┐
│ ItemInstance (500001)           │
│ - template_id: 10001            │
│ - container_id: 1001            │
│ - durability: 85                │
│ - enhance_level: 3              │
└─────────────────────────────────┘
         ↓ 存储在
┌─────────────────────────────────┐
│ ContainerInstance (1001)        │
│ - template_id: ct_treasure_chest│
│ - items: [500001, 500002, ...]  │
└─────────────────────────────────┘