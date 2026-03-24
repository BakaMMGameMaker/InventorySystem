#pragma once

#include "InventoryTypes.h"

// 背包
class InventoryRepository {
public:
    InventoryRepository() = default;

    // -------- 可堆叠物品 --------

    // 获取 物品 id => 存放该物品的所有格子 的映射
    const std::unordered_map<int, std::vector<StackEntry>> &GetAllStackEntries() const;
    // 存放指定 id 物品的所有格子
    std::vector<StackEntry> *FindStackEntries(int itemId);
    // 存放指定 id 物品的所有格子
    const std::vector<StackEntry> *FindStackEntries(int itemId) const;

    // 存放可堆叠物品的指定 id 的格子
    StackEntry *FindStackById(StackId stackId);
    // 存放可堆叠物品的指定 id 的格子
    const StackEntry *FindStackById(StackId stackId) const;

    // 新增一个条目 (格子)
    void AddStackEntry(const StackEntry &entry);
    // 删除指定 id 的格子
    bool RemoveStackEntry(StackId stackId);
    // 更新指定 id 的格子内的可堆叠物品的数量
    bool UpdateStackCount(StackId stackId, int newCount);
    // 设置指定 id 的格子的溢出状态
    bool SetStackOverflow(StackId stackId, bool isOverflow);

    // -------- 物品实例 --------

    // 获取 实例 id => 实例对象 的映射
    const std::unordered_map<InstanceId, ItemInstance> &GetAllInstances() const;
    // 指定 id 的实例对象
    ItemInstance *FindInstance(InstanceId instanceId);
    // 指定 id 的实例对象
    const ItemInstance *FindInstance(InstanceId instanceId) const;

    // 新增一个实例
    void AddInstance(const ItemInstance &instance);
    // 删除指定 id 的实例
    bool RemoveInstance(InstanceId instanceId);
    // 设置指定 id 的实例的溢出状态
    bool SetInstanceOverflow(InstanceId instanceId, bool isOverflow);

    // -------- id 生成 --------

    // 生成下一个存放可堆叠物品的格子的 id
    StackId GenerateNextStackId();
    // 生成下一个实例的 id，同时也可作为存放此实例的格子的 id
    InstanceId GenerateNextInstanceId();

private:
    std::unordered_map<int, std::vector<StackEntry>> m_stackEntriesByItemId; // 物品 id => 所有堆条目
    std::unordered_map<InstanceId, ItemInstance> m_instancesById;            // 实例 id => 所有实例条目

    StackId m_nextStackId = 1;       // 下一个堆的 id
    InstanceId m_nextInstanceId = 1; // 下一个物品实例的 id
};