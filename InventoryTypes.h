#pragma once

#include <algorithm>
#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using StackId = std::int64_t;
using InstanceId = std::int64_t;
using TimeStamp = std::int64_t;

// 主类别
enum class EItemMainCategory {
    Currency,   // 货币，摩拉、原石等
    Consumable, // 消耗品，经验书、煎蛋等
    Material,   // 突破材料，角色/武器突破素材
    QuestItem,  // 任务道具，钥匙、信件等
    Weapon,     // 武器
    Artifact,   // 圣遗物
    GiftPack    // 礼包
};

// 子类别
enum class EItemSubCategory {
    None, // 默认分类 / 未分类

    // Consumable
    HealFood,    // 生命回复
    ReviveFood,  // 角色复活
    StaminaFood, // 体力相关
    BuffFood,    // 增益效果，如提高攻击力、暴击率等
    Potion,      // 药剂

    // Material
    EnhancementMaterial, // 突破材料
    CharacterMaterial,   // 角色培养材料，经验书等
    WeaponMaterial,      // 武器培养材料
    TalentMaterial,      // 天赋培养材料

    // Quest
    QuestFunctional, // 任务类道具，如钥匙
    QuestArchive,    // 纪念类道具，如信件

    // Weapon
    Sword,    // 单手剑
    Claymore, // 双手剑
    Polearm,  // 长柄武器
    Bow,      // 弓
    Catalyst, // 法器

    // Artifact - 圣遗物，下面是各种槽位
    Flower,
    Plume,
    Sands,
    Goblet,
    Circlet
};

// 稀有度
enum class ERarity {
    None = 0,
    OneStar = 1, // 一星稀有度
    TwoStar = 2,
    ThreeStar = 3,
    FourStar = 4,
    FiveStar = 5 // 五星稀有度
};

// 货币类型
enum class ECurrencyType {
    Mora,    // 摩拉
    Primogem // 原石
};

// 背包类别
enum class EInventoryBucket {
    None,             // 无分类
    ConsumableBucket, // 消耗品
    MaterialBucket,   // 材料
    QuestBucket,      // 任务道具
    WeaponBucket,     // 武器
    ArtifactBucket,   // 圣遗物
    GiftPackBucket    // 礼包
};

// 条目类型
enum class EEntryType {
    StackEntry,   // 可堆叠物品
    InstanceEntry // 不可堆叠，即便同名也要单独管理
};

// 获取原因
enum class EAcquireReason {
    Pickup,       // 大世界拾取
    Reward,       // 任务奖励
    GiftPackOpen, // 礼包开启
    GM,           // 调试
    System        // 系统发放 / 版本补偿
};

// 溢出策略
enum class EOverflowPolicy {
    Disallow,     // 不允许溢出，背包满了就无法获得物品
    AllowOverflow // 允许溢出
};

// 排序选项
enum class ESortOption {
    Default,       // 默认排序
    Rarity,        // 稀有度
    Level,         // 等级
    Type,          // 类型
    AcquireTime,   // 获取时间
    Name,          // 名称
    EquippedState, // 装备状态
    LockedState    // 锁定状态
};

// 排序方向
enum class ESortDirection {
    Asc, // 升序
    Desc // 降序
};

// 快捷栏绑定
enum class EQuickbarBindType {
    None,  // 无绑定
    ItemId // 按照物品 id 绑定
};

// 物品使用结果
enum class EItemUseResult {
    Success,           // 成功
    InvalidItem,       // 无效物品
    NotUsable,         // 不可使用
    CountNotEnough,    // 数量不足
    InCooldown,        // 冷却中
    RestrictedByState, // 受到状态限制
    Failed             // 其他失败原因
};

// 获取结果
enum class EAcquireResult {
    Success,        // 成功获取
    InventoryFull,  // 背包满
    ReachItemLimit, // 达到物品堆叠上限
    InvalidConfig,  // 无效配置
    Failed          // 其他失败原因
};

// 删除结果
enum class EDeleteResult {
    Success,      // 成功
    NotFound,     // 未找到
    NotDeletable, // 不可删除
    Failed        // 其他失败原因
};

// 奖励条目
enum class ERewardEntryType {
    FixedItem,      // 固定物品奖励
    FixedCurrency,  // 固定货币
    RandomItemGroup // 随机奖励
};

// 背包页面
enum class EInventoryPage {
    Consumables, // 消耗品页
    Materials,   // 材料页
    QuestItems,  // 任务物品
    Weapons,     // 武器
    Artifacts,   // 圣遗物
    GiftPacks    // 礼包
};

// 物品配置
struct ItemConfig {
    int itemId = 0;                                                 // 物品 id
    std::string name;                                               // 物品名称
    EItemMainCategory mainCategory = EItemMainCategory::Consumable; // 物品主类别
    EItemSubCategory subCategory = EItemSubCategory::None;          // 物品子类别
    ERarity rarity = ERarity::None;                                 // 物品稀有度

    bool stackable = false; // 可堆叠
    int maxStack = 1;       // 堆数量上限

    bool canUse = false;         // 可使用
    bool canDelete = true;       // 可删除
    bool canDrag = true;         // 可拖拽
    bool canSort = true;         // 可排序
    bool canQuickbar = false;    // 可进入快捷栏
    bool showInInventory = true; // 是否在背包中显示

    EInventoryBucket bucket = EInventoryBucket::None; // 属于哪个背包桶 (圣遗物/武器/...)

    std::vector<std::string> tags; // 标签
};

// 随机奖励池子中的每一个候选
struct RandomRewardCandidate {
    int itemId = 0;   // 奖励的物品 id
    int minCount = 1; // 奖励的最小获得数量
    int maxCount = 1; // 奖励的最大获得数量
    int weight = 0;   // 奖励在池子中的权重
};

// 奖励条目
struct RewardEntryConfig {
    ERewardEntryType type = ERewardEntryType::FixedItem; // 奖励类别 (固定道具/货币/随机物品)

    int itemId = 0;   // 物品 id
    int minCount = 1; // 最小获得数量
    int maxCount = 1; // 最大获得数量

    ECurrencyType currencyType = ECurrencyType::Mora; // 奖励的货币类别
    std::int64_t currencyAmount = 0;                  // 奖励的货币数量

    std::vector<RandomRewardCandidate> candidates; // 候选物品
    int randomPickCount = 0;                       // 随机发放的物品的数量
};

// 礼包配置
struct GiftPackConfig {
    int itemId = 0;                         // 礼包的物品 id
    std::vector<RewardEntryConfig> rewards; // 礼包的奖励内容
};

// 货币存储，有多少摩拉和原石
struct CurrencyWallet {
    std::unordered_map<ECurrencyType, std::int64_t> balances;
};

// 存放可堆叠物品的一个格子
struct StackEntry {
    StackId stackId = 0; // 堆 id
    int itemId = 0;      // 当前堆存放的物品的 id
    int count = 0;       // 当前堆的物品数量

    bool isOverflow = false; // 当前堆是否溢出
    bool isNew = false;      // 是否新堆 (红点)

    TimeStamp createTime = 0;     // 获取时间 / 堆创建时间
    TimeStamp lastUpdateTime = 0; // 上次更新时间
};

// 实例对象
struct ItemInstance {
    InstanceId instanceId = 0; // 实例 id
    int itemId = 0;            // 物品 id

    bool isOverflow = false; // 是否溢出
    bool isNew = false;      // 是否新获取
    bool isLocked = false;   // 是否锁定

    int level = 1;           // 物品等级
    int refineLevel = 1;     // 精炼等级
    int strengthenLevel = 0; // 强化等级

    bool isEquipped = false;     // 是否已经装备
    int equippedCharacterId = 0; // 装备到谁身上

    TimeStamp createTime = 0; // 获取时间
};

// 单桶 (单页) 的状态
struct BucketCapacityState {
    EInventoryBucket bucket = EInventoryBucket::None;
    int baseCapacity = 0;      // 基础条目容量
    int normalUsedCount = 0;   // 非溢出的条目数量
    int overflowUsedCount = 0; // 溢出的条目数量

    // 当前桶正常 + 溢出的条目数量
    int TotalUsedCount() const { return normalUsedCount + overflowUsedCount; }

    // 当前桶中是否还有空闲的 Normal 格子
    bool HasNormalFreeSlot() const { return normalUsedCount < baseCapacity; }

    // 当前桶中还能容纳多少个 Normal 格子
    int NormalFreeSlotCount() const { return std::max(0, baseCapacity - normalUsedCount); }
};

// 背包每个桶 (每一页) 的状态
struct InventoryCapacityState {
    std::unordered_map<EInventoryBucket, BucketCapacityState> buckets;
};

// 一个条目 (格子) 的元数据
struct EntryKey {
    EEntryType entryType = EEntryType::StackEntry; // 存放可堆叠物品还是存放实例
    std::int64_t id = 0;                           // 格子的 id

    // 是否同一条目
    bool operator==(const EntryKey &rhs) const { return entryType == rhs.entryType && id == rhs.id; }
};

// 条目哈希
struct EntryKeyHash {
    std::size_t operator()(const EntryKey &key) const noexcept {
        const std::size_t h1 = std::hash<int>()(static_cast<int>(key.entryType));
        const std::size_t h2 = std::hash<std::int64_t>()(key.id);
        return h1 ^ (h2 + 0x9e3779b97f4a7c15ULL + (h1 << 6) + (h1 >> 2));
    }
};

// 条目展示出来的信息
struct InventoryEntryView {
    EntryKey key; // 条目基础信息

    int itemId = 0;          // 物品 id
    std::string displayName; // 展示名

    EItemMainCategory mainCategory = EItemMainCategory::Consumable; // 主类别
    EItemSubCategory subCategory = EItemSubCategory::None;          // 子类别
    ERarity rarity = ERarity::None;                                 // 稀有度

    int displayCount = 1; // 展示出来的数量
    int level = 0;        // 展示出来的等级

    bool isOverflow = false; // 是溢出状态
    bool isNew = false;      // 是新的条目
    bool isLocked = false;   // 是否锁定
    bool isEquipped = false; // 是否被装备

    int equippedCharacterId = 0; // 装备在哪个角色上

    std::vector<std::string> filterTags; // 标签

    int sortTypeKey = 0; // 排序键 ?
    std::int64_t sortTimeKey = 0;
};

// 过滤器
struct InventoryFilter {
    std::optional<EItemMainCategory> mainCategory; // 主类别过滤
    std::optional<EItemSubCategory> subCategory;   // 子类别过滤

    std::unordered_set<ERarity> rarities;               // 稀有度
    std::unordered_set<EItemSubCategory> subCategories; // 子类别

    std::optional<int> minLevel; // 最小等级
    std::optional<int> maxLevel; // 最大等级

    std::optional<bool> onlyLocked;     // 仅锁定
    std::optional<bool> onlyUnlocked;   // 仅非锁定
    std::optional<bool> onlyEquipped;   // 仅装备
    std::optional<bool> onlyUnequipped; // 仅非装备
    std::optional<bool> onlyOverflow;   // 仅溢出

    std::string keyword; // 关键词
};

// 背包排序规则
struct InventorySortSpec {
    ESortOption primary = ESortOption::Default;       // 主排序键
    ESortDirection primaryDir = ESortDirection::Desc; // 升降序

    ESortOption secondary = ESortOption::AcquireTime;   // 子排序键
    ESortDirection secondaryDir = ESortDirection::Desc; // 升降序
};

// 背包布局
struct InventoryLayoutState {
    EInventoryPage page = EInventoryPage::Consumables; // 哪个页面
    std::vector<EntryKey> visualOrder;                 // 物品表现出来的布局
    std::vector<EntryKey> manualSnapshot;              // 玩家拖动后保存的快照
    bool hasCustomLayout = false;                      // 是否使用自定义布局
};

// 快捷栏单个槽位
struct QuickbarSlot {
    int slotIndex = 0;                                    // 属于第几个快捷栏
    EQuickbarBindType bindType = EQuickbarBindType::None; // 绑定了什么
    int boundItemId = 0;                                  // 绑定到了哪个物品
    bool enabled = true;                                  // 是否启用
};

// 快捷栏状态
struct QuickbarState {
    static constexpr int kMaxSlots = 5;          // 最大快捷栏数量
    std::array<QuickbarSlot, kMaxSlots> slots{}; // 五个槽位
};

// 解算后的货币奖励
struct ResolvedCurrencyReward {
    ECurrencyType currencyType = ECurrencyType::Mora; // 货币类别
    std::int64_t amount = 0;                          // 奖励数量
};

// 解算后的物品奖励
struct ResolvedItemReward {
    int itemId = 0; // 物品 id
    int count = 0;  // 物品数量
};

// 解算后的奖励结果
struct ResolvedRewardResult {
    std::vector<ResolvedCurrencyReward> currencies; // 整个礼包的所有货币奖励
    std::vector<ResolvedItemReward> items;          // 整个礼包的所有物品奖励
};