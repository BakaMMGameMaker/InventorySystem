#pragma once

#include "InventoryCapacityService.h"
#include "InventoryRepository.h"
#include "ItemConfigService.h"

class InventoryRuleService {
public:
    InventoryRuleService(const ItemConfigService &configSvc, InventoryRepository &repo,
                         InventoryCapacityService &capacitySvc);

    // 是否可以删除指定 key 的物品
    bool CanDeleteItem(const EntryKey &key) const;
    // 是否可以使用指定 key 的物品
    bool CanUseItem(const EntryKey &key) const;
    // 是否可以把指定 key 的物品绑定到快捷栏
    bool CanBindToQuickbar(int itemId) const;
    // 是否可以拖拽指定 key 的物品
    bool CanDragItem(const EntryKey &key) const;

    // 指定 id 的任务道具是否被保护
    bool IsQuestItemProtected(int itemId) const;
    // 指定 id 的物品是否可以被消耗
    bool IsUsableConsumable(int itemId) const;

private:
    // 指定 key 的格子中存放的物体的配置信息
    const ItemConfig *FindConfigByEntry(const EntryKey &key) const;

private:
    const ItemConfigService &m_configSvc;
    InventoryRepository &m_repo;
    InventoryCapacityService &m_capacitySvc;
};