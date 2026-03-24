#pragma once

#include "InventoryTypes.h"

// 背包
class InventoryRepository {
public:
    InventoryRepository() = default;

    // -------- 可堆叠物品 --------
    const std::unordered_map<int, std::vector<StackEntry>> &GetAllStackEntries() const; // 获取所有可堆叠物品条目
    std::vector<StackEntry> *FindStackEntries(int itemId);                              // 所有物品 id 匹配的物品条目
    const std::vector<StackEntry> *FindStackEntries(int itemId) const;                  // 所有物品 id 匹配的物品条目

    StackEntry *FindStackById(StackId stackId);             // 根据条目 id 返回特定条目
    const StackEntry *FindStackById(StackId stackId) const; // 根据条目 id 返回特定条目

    void AddStackEntry(const StackEntry &entry);             // 新增堆条目
    bool RemoveStackEntry(StackId stackId);                  // 删除堆条目
    bool UpdateStackCount(StackId stackId, int newCount);    // 更新物品数量
    bool SetStackOverflow(StackId stackId, bool isOverflow); // 设置溢出状态

    // -------- 物品实例 --------
    const std::unordered_map<InstanceId, ItemInstance> &GetAllInstances() const; // 获取所有物品实例
    ItemInstance *FindInstance(InstanceId instanceId);                           // 根据实例 id 返回物品实例
    const ItemInstance *FindInstance(InstanceId instanceId) const;               // 根据实例 id 返回物品实例

    void AddInstance(const ItemInstance &instance);                   // 新增物品实例
    bool RemoveInstance(InstanceId instanceId);                       // 删除物品实例
    bool SetInstanceOverflow(InstanceId instanceId, bool isOverflow); // 设置物品实例溢出状态

    // -------- id 生成 --------
    StackId GenerateNextStackId();       // 生成新堆 id
    InstanceId GenerateNextInstanceId(); // 生成新物品实例 id

private:
    std::unordered_map<int, std::vector<StackEntry>> m_stackEntriesByItemId; // 物品 id => 所有堆条目
    std::unordered_map<InstanceId, ItemInstance> m_instancesById;            // 实例 id => 所有实例条目

    StackId m_nextStackId = 1;       // 下一个堆的 id
    InstanceId m_nextInstanceId = 1; // 下一个物品实例的 id
};