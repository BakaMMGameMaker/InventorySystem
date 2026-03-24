#include "InventoryRuleService.h"

InventoryRuleService::InventoryRuleService(const ItemConfigService &configSvc, InventoryRepository &repo,
                                           InventoryCapacityService &capacitySvc)
    : m_configSvc(configSvc), m_repo(repo), m_capacitySvc(capacitySvc) {}

bool InventoryRuleService::CanDeleteItem(const EntryKey &key) const {
    const ItemConfig *cfg = FindConfigByEntry(key);
    if (cfg == nullptr) { return false; }

    if (!cfg->canDelete) { return false; }

    // 不可以删除纪念性质的任务道具
    if (cfg->mainCategory == EItemMainCategory::QuestItem && cfg->subCategory == EItemSubCategory::QuestArchive) {
        return false;
    }

    if (key.entryType == EEntryType::InstanceEntry) {
        const ItemInstance *instance = m_repo.FindInstance(static_cast<InstanceId>(key.id));
        if (instance == nullptr) { return false; }

        if (instance->isEquipped) { return false; } // 物品 (如武器、圣遗物) 被装备，不可以删除
    }

    return true;
}

bool InventoryRuleService::CanUseItem(const EntryKey &key) const {
    const ItemConfig *cfg = FindConfigByEntry(key);
    if (cfg == nullptr) { return false; }

    if (!cfg->canUse) { return false; }

    if (key.entryType == EEntryType::StackEntry) {
        const StackEntry *entry = m_repo.FindStackById(static_cast<StackId>(key.id));
        return entry != nullptr && entry->count > 0; // 物品数量大于 0 才能使用
    }

    if (key.entryType == EEntryType::InstanceEntry) {
        const ItemInstance *instance = m_repo.FindInstance(static_cast<InstanceId>(key.id));
        return instance != nullptr;
    }

    return false;
}

bool InventoryRuleService::CanBindToQuickbar(int itemId) const {
    const ItemConfig *cfg = m_configSvc.GetItemConfig(itemId);
    if (cfg == nullptr) { return false; }

    if (!cfg->canQuickbar || !cfg->canUse) { return false; }

    if (!cfg->stackable) { return false; } // 只有可堆叠物品才能绑定到快捷栏

    if (cfg->mainCategory != EItemMainCategory::Consumable) { return false; } // 只有可消费物品才能绑定到快捷栏

    return true;
}

bool InventoryRuleService::CanDragItem(const EntryKey &key) const {
    const ItemConfig *cfg = FindConfigByEntry(key);
    if (cfg == nullptr) { return false; }

    return cfg->canDrag && cfg->showInInventory; // 可以拖拽且显示出来的物品才能被拖拽
}

bool InventoryRuleService::IsQuestItemProtected(int itemId) const {
    const ItemConfig *cfg = m_configSvc.GetItemConfig(itemId);
    if (cfg == nullptr) { return false; }

    return cfg->mainCategory == EItemMainCategory::QuestItem && !cfg->canDelete; // 是否为不可删除的任务道具
}

bool InventoryRuleService::IsUsableConsumable(int itemId) const {
    const ItemConfig *cfg = m_configSvc.GetItemConfig(itemId);
    if (cfg == nullptr) { return false; }

    return cfg->mainCategory == EItemMainCategory::Consumable && cfg->canUse; // 可以使用且为消耗品
}

const ItemConfig *InventoryRuleService::FindConfigByEntry(const EntryKey &key) const {
    if (key.entryType == EEntryType::StackEntry) {
        const StackEntry *stack = m_repo.FindStackById(static_cast<StackId>(key.id));
        if (stack == nullptr) { return nullptr; }
        return m_configSvc.GetItemConfig(stack->itemId);
    }

    if (key.entryType == EEntryType::InstanceEntry) {
        const ItemInstance *instance = m_repo.FindInstance(static_cast<InstanceId>(key.id));
        if (instance == nullptr) { return nullptr; }
        return m_configSvc.GetItemConfig(instance->itemId);
    }

    return nullptr;
}