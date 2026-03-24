#include "InventoryCapacityService.h"

#include <algorithm>
#include <vector>

InventoryCapacityService::InventoryCapacityService(InventoryRepository &repo, const ItemConfigService &configSvc)
    : m_repo(repo), m_configSvc(configSvc) {
    // 初始化每个桶的基础容量
    m_baseCapacities[EInventoryBucket::ConsumableBucket] = 2000;
    m_baseCapacities[EInventoryBucket::MaterialBucket] = 2000;
    m_baseCapacities[EInventoryBucket::QuestBucket] = 500;
    m_baseCapacities[EInventoryBucket::WeaponBucket] = 2000;
    m_baseCapacities[EInventoryBucket::ArtifactBucket] = 1500;
    m_baseCapacities[EInventoryBucket::GiftPackBucket] = 500;
}

void InventoryCapacityService::SetBaseCapacity(EInventoryBucket bucket, int capacity) {
    if (bucket == EInventoryBucket::None) { return; }
    m_baseCapacities[bucket] = std::max(0, capacity);
}

int InventoryCapacityService::GetBaseCapacity(EInventoryBucket bucket) const {
    auto it = m_baseCapacities.find(bucket);
    if (it == m_baseCapacities.end()) { return 0; }
    return it->second;
}

InventoryCapacityState InventoryCapacityService::BuildCapacityState() const {
    InventoryCapacityState state;

    for (const auto &[bucket, capacity] : m_baseCapacities) {
        BucketCapacityState bucketState;
        bucketState.bucket = bucket;
        bucketState.baseCapacity = capacity;
        bucketState.normalUsedCount =
            CountNormalStackEntriesInBucket(bucket) + CountNormalInstanceEntriesInBucket(bucket);
        bucketState.overflowUsedCount =
            CountOverflowStackEntriesInBucket(bucket) + CountOverflowInstanceEntriesInBucket(bucket);

        state.buckets[bucket] = bucketState;
    }

    return state;
}

BucketCapacityState InventoryCapacityService::GetBucketState(EInventoryBucket bucket) const {
    BucketCapacityState state;
    state.bucket = bucket;
    state.baseCapacity = GetBaseCapacity(bucket);
    state.normalUsedCount = CountNormalStackEntriesInBucket(bucket) + CountNormalInstanceEntriesInBucket(bucket);
    state.overflowUsedCount = CountOverflowStackEntriesInBucket(bucket) + CountOverflowInstanceEntriesInBucket(bucket);
    return state;
}

bool InventoryCapacityService::CanAcquireNormally(int itemId, int count) const {
    return CanAcquireWithPolicy(itemId, count, EOverflowPolicy::Disallow);
}

bool InventoryCapacityService::CanAcquireWithPolicy(int itemId, int count, EOverflowPolicy policy) const {
    if (count <= 0) { return false; }

    const ItemConfig *cfg = m_configSvc.GetItemConfig(itemId);
    if (cfg == nullptr) { return false; }

    // 如果物品是货币，返回 true
    if (cfg->mainCategory == EItemMainCategory::Currency) { return true; }

    // 如果即将到达物品上限，如提瓦特煎蛋已经有 9998 个，还想获取 3 个，返回 false
    if (WillReachItemLimit(itemId, count)) { return false; }

    // 如果增添 count 个指定 id 的物品不会创建新的条目，返回 true
    if (!WillRequireNewNormalEntry(itemId, count)) { return true; }

    const EInventoryBucket bucket = ResolveBucket(itemId); // 指定 id 的物品应该存放的桶
    if (bucket == EInventoryBucket::None) { return false; }

    const BucketCapacityState bucketState = GetBucketState(bucket);
    if (bucketState.HasNormalFreeSlot()) { return true; } // 还有空闲的格子

    return policy == EOverflowPolicy::AllowOverflow;
}

bool InventoryCapacityService::WillRequireNewNormalEntry(int itemId, int count) const {
    if (count <= 0) { return false; }

    const ItemConfig *cfg = m_configSvc.GetItemConfig(itemId);
    if (cfg == nullptr) { return false; }

    // 如果是货币，不需要新格子
    if (cfg->mainCategory == EItemMainCategory::Currency) { return false; }

    // 如果物品不可堆叠，需要新格子
    if (!cfg->stackable) { return true; }

    // 找到存放当前物品的所有格子
    const std::vector<StackEntry> *entries = m_repo.FindStackEntries(itemId);
    // 如果没有存放当前物品的格子，返回真
    if (entries == nullptr || entries->empty()) { return true; }

    // 看看剩余空间能否消费完所需的数量
    int remain = count;
    for (const StackEntry &entry : *entries) {
        if (entry.isOverflow) { continue; } // 溢出的格子不能再塞

        const int space = std::max(0, cfg->maxStack - entry.count);
        remain -= std::min(space, remain);
        if (remain <= 0) { return false; } // 可以消费完需求数量，无需开新格子
    }

    return true;
}

bool InventoryCapacityService::WillReachItemLimit(int itemId, int count) const {
    if (count <= 0) { return false; }

    const ItemConfig *cfg = m_configSvc.GetItemConfig(itemId);
    if (cfg == nullptr) { return true; }

    // 如果是货币类型，不会到达上限
    if (cfg->mainCategory == EItemMainCategory::Currency) { return false; }

    // 如果是实例物品，不会到达上限
    if (!cfg->stackable) { return false; }

    // 统计当前物品非溢出状态下总数量
    int normalTotal = 0;
    const std::vector<StackEntry> *entries = m_repo.FindStackEntries(itemId);
    if (entries != nullptr) {
        for (const StackEntry &entry : *entries) {
            if (!entry.isOverflow) { normalTotal += entry.count; }
        }
    }

    // 还能不能塞下 count 个指定 id 的物品
    return normalTotal + count > cfg->maxStack;
}

void InventoryCapacityService::NormalizeOverflow() {
    for (const auto &[bucket, baseCapacity] : m_baseCapacities) {
        (void)baseCapacity;

        BucketCapacityState state = GetBucketState(bucket);
        int freeSlots = state.NormalFreeSlotCount();                      // 当前桶剩余的空闲格子数
        if (freeSlots <= 0 || state.overflowUsedCount <= 0) { continue; } // 当前桶没有空闲格子或者未发生溢出，跳过

        struct OverflowCandidate {
            bool isStack = false;
            std::int64_t id = 0;
            TimeStamp createTime = 0;
        };

        std::vector<OverflowCandidate> candidates;

        for (const auto &[itemId, entries] : m_repo.GetAllStackEntries()) {
            const ItemConfig *cfg = m_configSvc.GetItemConfig(itemId);
            if (cfg == nullptr || cfg->bucket != bucket) { continue; } // 配置不存在或者物品不属于当前桶

            for (const StackEntry &entry : entries) {
                if (entry.isOverflow) { candidates.push_back({true, entry.stackId, entry.createTime}); }
            }
        }

        for (const auto &[instanceId, instance] : m_repo.GetAllInstances()) {
            const ItemConfig *cfg = m_configSvc.GetItemConfig(instance.itemId);
            if (cfg == nullptr || cfg->bucket != bucket) { continue; }

            if (instance.isOverflow) { candidates.push_back({false, instanceId, instance.createTime}); }
        }

        // 根据创建时间和物品 id 对溢出候选进行排序
        std::sort(candidates.begin(), candidates.end(), [](const OverflowCandidate &lhs, const OverflowCandidate &rhs) {
            if (lhs.createTime != rhs.createTime) { return lhs.createTime < rhs.createTime; }
            return lhs.id < rhs.id;
        });

        for (const OverflowCandidate &candidate : candidates) {
            if (freeSlots <= 0) { break; } // 如果已经消费完了所有空闲格子

            if (candidate.isStack) {
                // 消费一个空闲格子，把原本发生溢出的格子的溢出位设为 false
                if (m_repo.SetStackOverflow(candidate.id, false)) { --freeSlots; }
            } else {
                if (m_repo.SetInstanceOverflow(candidate.id, false)) { --freeSlots; }
            }
        }
    }
}

EInventoryBucket InventoryCapacityService::ResolveBucket(int itemId) const {
    const ItemConfig *cfg = m_configSvc.GetItemConfig(itemId);
    if (cfg == nullptr) { return EInventoryBucket::None; }
    return cfg->bucket;
}

bool InventoryCapacityService::IsStackableItem(int itemId) const {
    const ItemConfig *cfg = m_configSvc.GetItemConfig(itemId);
    return cfg != nullptr && cfg->stackable;
}

int InventoryCapacityService::CountNormalStackEntriesInBucket(EInventoryBucket bucket) const {
    int count = 0;

    for (const auto &[itemId, entries] : m_repo.GetAllStackEntries()) {
        const ItemConfig *cfg = m_configSvc.GetItemConfig(itemId);
        if (cfg == nullptr || cfg->bucket != bucket) { continue; }

        for (const StackEntry &entry : entries) {
            if (!entry.isOverflow) { ++count; }
        }
    }

    return count;
}

int InventoryCapacityService::CountOverflowStackEntriesInBucket(EInventoryBucket bucket) const {
    int count = 0;

    for (const auto &[itemId, entries] : m_repo.GetAllStackEntries()) {
        const ItemConfig *cfg = m_configSvc.GetItemConfig(itemId);
        if (cfg == nullptr || cfg->bucket != bucket) { continue; }

        for (const StackEntry &entry : entries) {
            if (entry.isOverflow) { ++count; }
        }
    }

    return count;
}

int InventoryCapacityService::CountNormalInstanceEntriesInBucket(EInventoryBucket bucket) const {
    int count = 0;

    for (const auto &[instanceId, instance] : m_repo.GetAllInstances()) {
        (void)instanceId;
        const ItemConfig *cfg = m_configSvc.GetItemConfig(instance.itemId);
        if (cfg == nullptr || cfg->bucket != bucket) { continue; }

        if (!instance.isOverflow) { ++count; }
    }

    return count;
}

int InventoryCapacityService::CountOverflowInstanceEntriesInBucket(EInventoryBucket bucket) const {
    int count = 0;

    for (const auto &[instanceId, instance] : m_repo.GetAllInstances()) {
        (void)instanceId;
        const ItemConfig *cfg = m_configSvc.GetItemConfig(instance.itemId);
        if (cfg == nullptr || cfg->bucket != bucket) { continue; }

        if (instance.isOverflow) { ++count; }
    }

    return count;
}