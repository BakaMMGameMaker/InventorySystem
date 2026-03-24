# 仿原神混合背包系统

## 1 - 整体设计思路

接近原神风格、同时加入一些扩展特性的混合式背包系统

### 支持不同物品形态

- 货币：摩拉、原石
- 可堆叠道具：料理、药剂、材料、礼包
- 不可堆叠实例物品：武器、圣遗物、纪念道具

### 支持分类展示

- 消耗品
- 材料
- 任务道具
- 武器
- 圣遗物
- 礼包

### 支持排序、筛选、快捷物品栏

- 按稀有度、等级、类型、装备状态等排序
- 按类别、稀有度、等级、关键词等筛选
- 可将消耗品绑定到快捷栏快速使用

### 支持玩家手动拖拽布局

- 平时不自动打乱用户布局
- 只有用户主动点击排序时才重排
- 可恢复手动布局

### 支持礼包突破容量上限

- 普通拾取严格受容量限制
- 礼包开启时允许生成 overflow 物品
- 后续腾出空间后自动转正

### 基本理念

- 底层按“货币 / 堆叠物品 / 实例物品”分开存储；
- 中层按规则、容量、命令、查询等职责分层；
- 上层通过 Controller 聚合给 UI 使用

## 2 - 系统层级

### 配置层

负责描述“物品是什么”，包括：
- ItemConfig
- GiftPackConfig
- RewardEntryConfig

### 数据存储层

负责描述“玩家当前拥有什么”，包括：
- CurrencyWallet
- StackEntry
- ItemInstance
- InventoryRepository

### 规则与能力层

负责回答“能不能做”和“容量是否允许”，包括：
- InventoryCapacityService
- InventoryRuleService
- RewardResolveService

### 命令与查询层

负责“真正执行修改”和“为 UI 组织数据”，包括：
- InventoryCommandService
- InventoryQueryService
- InventoryLayoutService
- QuickbarService

### 应用协调层

负责把多个服务组合成易用的统一入口，包括：
- InventoryController

## 3 - 模块划分与分工

### 模块 1：InventoryTypes / Config Types

作用：定义整个背包系统的基础类型和数据结构

包括：
- 枚举：
  - EItemMainCategory
  - EItemSubCategory
  - ERarity
  - ECurrencyType
  - EInventoryBucket
  - EInventoryPage
  - ESortOption
  - EOverflowPolicy
  - EAcquireResult

- 数据结构：
  - ItemConfig
  - GiftPackConfig
  - StackEntry
  - ItemInstance
  - InventoryEntryView
  - InventoryFilter
  - InventorySortSpec
  - QuickbarSlot
  - InventoryLayoutState

### 模块 2：ItemConfigService

作用：管理静态配置，提供查询接口

负责：
  - 通过 itemId 获取 ItemConfig
  - 通过礼包 itemId 获取 GiftPackConfig
  - 判断某个物品是不是礼包
  - 判断某个物品是不是货币物品

### 模块 3：InventoryRepository

作用：玩家背包的真实持有仓库

负责：
  - 存储可堆叠物品 StackEntry
  - 存储实例物品 ItemInstance
  - 生成唯一 stackId / instanceId
  - 提供增删改查接口

### 模块 4：WalletService

作用：管理货币

负责：
  - 查询摩拉 / 原石余额
  - 增加货币
  - 消耗货币

### 模块 5：InventoryCapacityService

作用：管理容量规则和溢出逻辑

负责：
  - 维护各 bucket 的基础容量
  - 统计正常占用和 overflow 占用
  - 判断物品能否通过普通路径获得
  - 判断物品能否通过 AllowOverflow 获得
  - 判断是否需要新建条目
  - 判断是否达到普通 item 上限
  - 在空间释放后将 overflow 自动转正

### 模块 6：InventoryRuleService

作用：管理业务规则

负责：
  - 能否删除
  - 能否使用
  - 能否绑定快捷栏
  - 能否拖拽
  - 任务物品是否受保护
  - 某物品是否属于可用消耗品

### 模块 7：RewardResolveService

作用：解析礼包奖励

负责：
  - 读取礼包配置
  - 生成礼包开启后的实际奖励列表
  - 支持固定奖励
  - 支持随机奖励组

### 模块 8：InventoryCommandService

作用：背包系统的执行引擎

负责：
  - 普通拾取 PickupItem
  - 通用获得 AcquireItem
  - 获得实例物品 AcquireInstanceItem
  - 使用条目 UseEntry
  - 删除条目 DeleteEntry
  - 开礼包 OpenGiftPack

内部逻辑：
  - 调用 RuleService 做合法性判断
  - 调用 CapacityService 做容量校验
  - 调用 RewardResolveService 解析礼包
  - 调用 Repository / WalletService 真正写数据
  - 调用 NormalizeOverflow() 自动消解溢出

### 模块 9：InventoryQueryService

作用：为 UI 构建展示数据

负责：
  - 按页签构建 InventoryEntryView
  - 把 StackEntry / ItemInstance 转成统一展示模型
  - 应用筛选
  - 应用排序

### 模块 10：InventoryLayoutService

作用：管理玩家自定义视觉布局

负责：
  - 保存某页签的 visualOrder
  - 支持拖拽重排
  - 保存最近一次手动布局快照
  - 恢复手动布局

### 模块 11：QuickbarService

作用：管理快捷栏

负责：
  - 把某个 itemId 绑定到快捷栏槽位
  - 解绑
  - 使用槽位对应物品
  - 优先消费 normal stack，再消费 overflow stack

### 模块 12：InventoryController

作用：系统统一入口，对外暴露易用接口

负责：
  - 聚合所有 service
  - 暴露配置、仓库、钱包、命令、查询、布局、快捷栏接口
  - 提供更方便的上层调用，例如：
      - GetPageEntries()
      - SortPage()
      - MoveEntryOnPage()
      - BindQuickbarItem()
      - UseQuickbarSlot()

## 4 - 关键设计决策

### 货币与普通背包分离

原因：
  - 不占容量
  - 不参与页签
  - 不参与拖拽
  - 不参与快捷栏

### 堆叠物品与实例物品分离

原因：
  - 武器、圣遗物必须有唯一 ID
  - 可堆叠物品更适合按 stack 管理
  - overflow 时可堆叠物品可以拆成多个 stack entry

### 容量按 bucket 管理

原因：
  - 武器、圣遗物、道具页容量应独立
  - UI 提示更清晰

### 普通获取与礼包获取分策略

通过：
  - Disallow
  - AllowOverflow

### 查询与修改分离

- CommandService 只负责改
- QueryService 只负责读

### 布局只影响显示

- 玩家拖拽不改变背包真实存储结构，只影响视觉顺序