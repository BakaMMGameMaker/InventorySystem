#include "InventoryCommandService.h"

#include <algorithm>
#include <vector>

InventoryCommandService::InventoryCommandService(ItemConfigService &configSvc, InventoryRepository &repo,
                                                 WalletService &walletSvc, InventoryCapacityService &capacitySvc,
                                                 InventoryRuleService &ruleSvc, RewardResolveService &rewardSvc)
    : m_configSvc(configSvc), m_repo(repo), m_walletSvc(walletSvc), m_capacitySvc(capacitySvc), m_ruleSvc(ruleSvc),
      m_rewardSvc(rewardSvc) {}

EAcquireResult InventoryCommandService::PickupItem(int itemId, int count) {
    if (count <= 0) { return EAcquireResult::Failed; }

    // 不允许溢出
    return AcquireItem(itemId, count, EAcquireReason::Pickup, EOverflowPolicy::Disallow);
}

EAcquireResult InventoryCommandService::AcquireItem(int itemId, int count, EAcquireReason reason,
                                                    EOverflowPolicy overflowPolicy) {
    (void)reason;

    if (count <= 0) { return EAcquireResult::Failed; }

    const ItemConfig *cfg = m_configSvc.GetItemConfig(itemId);
    if (cfg == nullptr) { return EAcquireResult::InvalidConfig; }

    if (cfg->mainCategory == EItemMainCategory::Currency) { return AcquireCurrencyItem(itemId, count); }

    if (cfg->stackable) { return AcquireStackableItemInternal(itemId, count, overflowPolicy); }

    return AcquireNonStackableItemInternal(itemId, count, overflowPolicy);
}

EAcquireResult InventoryCommandService::AcquireInstanceItem(const ItemInstance &instance, EAcquireReason reason,
                                                            EOverflowPolicy overflowPolicy) {
    (void)reason;

    const ItemConfig *cfg = m_configSvc.GetItemConfig(instance.itemId);
    if (cfg == nullptr) { return EAcquireResult::InvalidConfig; }

    if (cfg->mainCategory == EItemMainCategory::Currency || cfg->stackable) { return EAcquireResult::Failed; }

    if (!m_capacitySvc.CanAcquireWithPolicy(instance.itemId, 1, overflowPolicy)) {
        if (m_capacitySvc.WillReachItemLimit(instance.itemId, 1)) { return EAcquireResult::ReachItemLimit; }
        return EAcquireResult::InventoryFull;
    }

    ItemInstance newInstance = instance;
    if (newInstance.instanceId == 0) { newInstance.instanceId = m_repo.GenerateNextInstanceId(); }

    const bool canNormalAcquire = m_capacitySvc.CanAcquireNormally(instance.itemId, 1);
    newInstance.isOverflow = !canNormalAcquire && overflowPolicy == EOverflowPolicy::AllowOverflow;

    m_repo.AddInstance(newInstance);
    return EAcquireResult::Success;
}

EItemUseResult InventoryCommandService::UseEntry(const EntryKey &key, int count) {
    if (count <= 0) { return EItemUseResult::Failed; }

    if (!m_ruleSvc.CanUseItem(key)) { return EItemUseResult::NotUsable; }

    if (key.entryType == EEntryType::StackEntry) {
        StackEntry *entry = m_repo.FindStackById(static_cast<StackId>(key.id));
        if (entry == nullptr) { return EItemUseResult::InvalidItem; }

        const ItemConfig *cfg = m_configSvc.GetItemConfig(entry->itemId);
        if (cfg == nullptr) { return EItemUseResult::InvalidItem; }

        if (entry->count < count) { return EItemUseResult::CountNotEnough; }

        if (!ConsumeStackEntry(entry->stackId, count)) { return EItemUseResult::Failed; }

        m_capacitySvc.NormalizeOverflow();

        if (cfg->mainCategory == EItemMainCategory::GiftPack) { return EItemUseResult::Success; }

        return EItemUseResult::Success;
    }

    if (key.entryType == EEntryType::InstanceEntry) {
        ItemInstance *instance = m_repo.FindInstance(static_cast<InstanceId>(key.id));
        if (instance == nullptr) { return EItemUseResult::InvalidItem; }

        if (!ConsumeInstance(instance->instanceId)) { return EItemUseResult::Failed; }

        m_capacitySvc.NormalizeOverflow();
        return EItemUseResult::Success;
    }

    return EItemUseResult::Failed;
}

EDeleteResult InventoryCommandService::DeleteEntry(const EntryKey &key, int count) {
    if (count <= 0) { return EDeleteResult::Failed; }

    if (!m_ruleSvc.CanDeleteItem(key)) { return EDeleteResult::NotDeletable; }

    bool success = false;

    if (key.entryType == EEntryType::StackEntry) {
        StackEntry *entry = m_repo.FindStackById(static_cast<StackId>(key.id));
        if (entry == nullptr) { return EDeleteResult::NotFound; }

        if (entry->count < count) { return EDeleteResult::Failed; }

        success = ConsumeStackEntry(entry->stackId, count);
    } else if (key.entryType == EEntryType::InstanceEntry) {
        success = ConsumeInstance(static_cast<InstanceId>(key.id));
        if (!success) { return EDeleteResult::NotFound; }
    }

    if (!success) { return EDeleteResult::Failed; }

    m_capacitySvc.NormalizeOverflow();
    return EDeleteResult::Success;
}

EItemUseResult InventoryCommandService::OpenGiftPack(const EntryKey &key) {
    if (key.entryType != EEntryType::StackEntry) { return EItemUseResult::InvalidItem; }

    StackEntry *entry = m_repo.FindStackById(static_cast<StackId>(key.id));
    if (entry == nullptr) { return EItemUseResult::InvalidItem; }

    const ItemConfig *cfg = m_configSvc.GetItemConfig(entry->itemId);
    if (cfg == nullptr) { return EItemUseResult::InvalidItem; }

    if (cfg->mainCategory != EItemMainCategory::GiftPack) { return EItemUseResult::NotUsable; }

    if (!m_ruleSvc.CanUseItem(key)) { return EItemUseResult::NotUsable; }

    if (entry->count <= 0) { return EItemUseResult::CountNotEnough; }

    const ResolvedRewardResult resolved = m_rewardSvc.ResolveGiftPackRewards(entry->itemId);

    ApplyResolvedRewards(resolved);

    if (!ConsumeStackEntry(entry->stackId, 1)) { return EItemUseResult::Failed; }

    m_capacitySvc.NormalizeOverflow();
    return EItemUseResult::Success;
}

EAcquireResult InventoryCommandService::AcquireCurrencyItem(int itemId, int count) {
    const ECurrencyType currencyType = ResolveCurrencyTypeByItemId(itemId);
    m_walletSvc.AddCurrency(currencyType, static_cast<std::int64_t>(count));
    return EAcquireResult::Success;
}

EAcquireResult InventoryCommandService::AcquireStackableItemInternal(int itemId, int count,
                                                                     EOverflowPolicy overflowPolicy) {
    if (count <= 0) { return EAcquireResult::Failed; }

    const ItemConfig *cfg = m_configSvc.GetItemConfig(itemId);
    if (cfg == nullptr) { return EAcquireResult::InvalidConfig; }

    int remain = count;

    // 1. 优先填 normal stacks
    std::vector<StackEntry> *entries = m_repo.FindStackEntries(itemId);
    if (entries != nullptr) {
        for (StackEntry &entry : *entries) {
            if (remain <= 0) { break; }

            if (entry.isOverflow) { continue; }

            const int canAdd = std::max(0, cfg->maxStack - entry.count);
            if (canAdd <= 0) { continue; }

            const int addNum = std::min(canAdd, remain);
            entry.count += addNum;
            remain -= addNum;
        }
    }

    if (remain <= 0) { return EAcquireResult::Success; }

    // 2. 还需要创建新条目
    if (!m_capacitySvc.CanAcquireWithPolicy(itemId, remain, overflowPolicy)) {
        if (m_capacitySvc.WillReachItemLimit(itemId, remain)) { return EAcquireResult::ReachItemLimit; }
        return EAcquireResult::InventoryFull;
    }

    const bool canNormalAcquire = m_capacitySvc.CanAcquireNormally(itemId, remain);
    const bool createOverflow = !canNormalAcquire && overflowPolicy == EOverflowPolicy::AllowOverflow;

    if (!createOverflow) {
        // 普通路径：只允许新建一个 normal stack，且总 normal 数量不能超过 maxStack
        StackEntry newEntry;
        newEntry.stackId = m_repo.GenerateNextStackId();
        newEntry.itemId = itemId;
        newEntry.count = remain;
        newEntry.isOverflow = false;
        m_repo.AddStackEntry(newEntry);
        return EAcquireResult::Success;
    }

    // 3. overflow 路径：允许把剩余内容拆成 overflow stacks
    while (remain > 0) {
        const int chunk = std::min(cfg->maxStack, remain);

        StackEntry overflowEntry;
        overflowEntry.stackId = m_repo.GenerateNextStackId();
        overflowEntry.itemId = itemId;
        overflowEntry.count = chunk;
        overflowEntry.isOverflow = true;
        m_repo.AddStackEntry(overflowEntry);

        remain -= chunk;
    }

    return EAcquireResult::Success;
}

EAcquireResult InventoryCommandService::AcquireNonStackableItemInternal(int itemId, int count,
                                                                        EOverflowPolicy overflowPolicy) {
    if (count <= 0) { return EAcquireResult::Failed; }

    const ItemConfig *cfg = m_configSvc.GetItemConfig(itemId);
    if (cfg == nullptr) { return EAcquireResult::InvalidConfig; }

    for (int i = 0; i < count; ++i) {
        if (!m_capacitySvc.CanAcquireWithPolicy(itemId, 1, overflowPolicy)) {
            if (m_capacitySvc.WillReachItemLimit(itemId, 1)) { return EAcquireResult::ReachItemLimit; }
            return EAcquireResult::InventoryFull;
        }

        const bool canNormalAcquire = m_capacitySvc.CanAcquireNormally(itemId, 1);

        ItemInstance instance;
        instance.instanceId = m_repo.GenerateNextInstanceId();
        instance.itemId = itemId;
        instance.isOverflow = !canNormalAcquire && overflowPolicy == EOverflowPolicy::AllowOverflow;

        if (cfg->mainCategory == EItemMainCategory::Weapon) {
            instance.level = 1;
            instance.refineLevel = 1;
        } else if (cfg->mainCategory == EItemMainCategory::Artifact) {
            instance.level = 0;
            instance.strengthenLevel = 0;
        }

        m_repo.AddInstance(instance);
    }

    return EAcquireResult::Success;
}

bool InventoryCommandService::ConsumeStackEntry(StackId stackId, int count) {
    if (count <= 0) { return false; }

    StackEntry *entry = m_repo.FindStackById(stackId);
    if (entry == nullptr || entry->count < count) { return false; } // Stack 失效或物品数量不足

    entry->count -= count;
    if (entry->count == 0) { return m_repo.RemoveStackEntry(stackId); } // Stack 空了

    return true; // 消耗成功
}

bool InventoryCommandService::ConsumeInstance(InstanceId instanceId) { return m_repo.RemoveInstance(instanceId); }

void InventoryCommandService::ApplyResolvedRewards(const ResolvedRewardResult &rewards) {
    for (const ResolvedCurrencyReward &currencyReward : rewards.currencies) {
        if (currencyReward.amount > 0) { m_walletSvc.AddCurrency(currencyReward.currencyType, currencyReward.amount); }
    }

    for (const ResolvedItemReward &itemReward : rewards.items) {
        if (itemReward.itemId <= 0 || itemReward.count <= 0) { continue; }

        const ItemConfig *cfg = m_configSvc.GetItemConfig(itemReward.itemId);
        if (cfg == nullptr) { continue; }

        if (cfg->mainCategory == EItemMainCategory::Currency) {
            AcquireCurrencyItem(itemReward.itemId, itemReward.count);
        } else {
            AcquireItem(itemReward.itemId, itemReward.count, EAcquireReason::GiftPackOpen,
                        EOverflowPolicy::AllowOverflow); // 开启礼包允许溢出
        }
    }
}

ECurrencyType InventoryCommandService::ResolveCurrencyTypeByItemId(int itemId) const {
    // 当前按样例配置约定：
    // 1 -> Mora
    // 2 -> Primogem
    switch (itemId) {
    case 1:
        return ECurrencyType::Mora;
    case 2:
        return ECurrencyType::Primogem;
    default:
        return ECurrencyType::Mora;
    }
}