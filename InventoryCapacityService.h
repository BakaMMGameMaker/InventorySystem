#pragma once

#include "InventoryRepository.h"
#include "ItemConfigService.h"

class InventoryCapacityService {
public:
    InventoryCapacityService(InventoryRepository &repo, const ItemConfigService &configSvc);

    // 设置指定桶的容量
    void SetBaseCapacity(EInventoryBucket bucket, int capacity);
    // 获取指定桶的容量
    int GetBaseCapacity(EInventoryBucket bucket) const;

    // 构造当前整个背包每一个桶的状态信息
    InventoryCapacityState BuildCapacityState() const;
    // 获取指定桶的当前状态
    BucketCapacityState GetBucketState(EInventoryBucket bucket) const;

    // 指定 id 的物品在不允许溢出的条件下是否还能增加 count 个
    bool CanAcquireNormally(int itemId, int count) const;
    // 指定 id 的物品在给定溢出策略下是否还能增加 count 个
    bool CanAcquireWithPolicy(int itemId, int count, EOverflowPolicy policy) const;

    // 指定 id 的物品新增 count 个后是否需要新的格子
    bool WillRequireNewNormalEntry(int itemId, int count) const;
    // 指定 id 的物品新增 count 个是否会到达上限
    bool WillReachItemLimit(int itemId, int count) const;

    // 根据空闲槽位数量将溢出状态的 Stack 或者 Instance 按顺序转回 Normal 状态 （不会把溢出物品合并到未满 Stack 中）
    void NormalizeOverflow();

private:
    // 解析指定 id 的物品应该在哪个桶 (page)
    EInventoryBucket ResolveBucket(int itemId) const;
    // 指定 id 的物品是否为可堆叠物品
    bool IsStackableItem(int itemId) const;

    // 指定桶中未发生溢出的存放可堆叠物品的格子数
    int CountNormalStackEntriesInBucket(EInventoryBucket bucket) const;
    // 指定桶中已发生溢出的存放可堆叠物品的格子数
    int CountOverflowStackEntriesInBucket(EInventoryBucket bucket) const;
    // 指定桶中未发生溢出的实例数量
    int CountNormalInstanceEntriesInBucket(EInventoryBucket bucket) const;
    // 指定桶中已发生溢出的实例数量
    int CountOverflowInstanceEntriesInBucket(EInventoryBucket bucket) const;

private:
    InventoryRepository &m_repo;
    const ItemConfigService &m_configSvc;
    std::unordered_map<EInventoryBucket, int> m_baseCapacities;
};