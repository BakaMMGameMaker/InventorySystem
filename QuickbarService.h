#pragma once

#include "InventoryCommandService.h"
#include "InventoryRepository.h"
#include "InventoryRuleService.h"

// 快捷栏
class QuickbarService {
public:
    QuickbarService(InventoryCommandService &commandSvc, InventoryRuleService &ruleSvc, InventoryRepository &repo);

    // 绑定指定 index 的槽位到指定 id 的物品
    bool BindItemId(int slotIndex, int itemId);
    // 解绑指定 index 的槽位
    bool Unbind(int slotIndex);

    // 使用指定 index 的快捷栏对应的物品
    EItemUseResult UseSlot(int slotIndex);

    // 获取整个快捷栏的状态
    const QuickbarState &GetState() const;

private:
    // 槽位 index 是否合法
    bool IsValidSlotIndex(int slotIndex) const;
    // 寻找一个可用的物品堆
    StackEntry *FindUsableStackByItemId(int itemId) const;

private:
    QuickbarState m_state;
    InventoryCommandService &m_commandSvc;
    InventoryRuleService &m_ruleSvc;
    InventoryRepository &m_repo;
};