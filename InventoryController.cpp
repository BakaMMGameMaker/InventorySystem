#include "InventoryController.h"
#include "InventoryTypes.h"

#include <unordered_map>
#include <unordered_set>

InventoryController::InventoryController()
    : m_configSvc(), m_repo(), m_walletSvc(), m_capacitySvc(m_repo, m_configSvc),
      m_ruleSvc(m_configSvc, m_repo, m_capacitySvc), m_rewardSvc(m_configSvc),
      m_commandSvc(m_configSvc, m_repo, m_walletSvc, m_capacitySvc, m_ruleSvc, m_rewardSvc),
      m_querySvc(m_configSvc, m_repo), m_layoutSvc(), m_quickbarSvc(m_commandSvc, m_ruleSvc, m_repo) {
    InitializeDefaultCapacities();
}

// ----- Accessors -----

ItemConfigService &InventoryController::GetConfigService() { return m_configSvc; }

InventoryRepository &InventoryController::GetRepository() { return m_repo; }

WalletService &InventoryController::GetWalletService() { return m_walletSvc; }

InventoryCapacityService &InventoryController::GetCapacityService() { return m_capacitySvc; }

InventoryRuleService &InventoryController::GetRuleService() { return m_ruleSvc; }

InventoryCommandService &InventoryController::GetCommandService() { return m_commandSvc; }

InventoryQueryService &InventoryController::GetQueryService() { return m_querySvc; }

InventoryLayoutService &InventoryController::GetLayoutService() { return m_layoutSvc; }

QuickbarService &InventoryController::GetQuickbarService() { return m_quickbarSvc; }

const ItemConfigService &InventoryController::GetConfigService() const { return m_configSvc; }

const InventoryRepository &InventoryController::GetRepository() const { return m_repo; }

const WalletService &InventoryController::GetWalletService() const { return m_walletSvc; }

const InventoryCapacityService &InventoryController::GetCapacityService() const { return m_capacitySvc; }

const InventoryRuleService &InventoryController::GetRuleService() const { return m_ruleSvc; }

const InventoryCommandService &InventoryController::GetCommandService() const { return m_commandSvc; }

const InventoryQueryService &InventoryController::GetQueryService() const { return m_querySvc; }

const InventoryLayoutService &InventoryController::GetLayoutService() const { return m_layoutSvc; }

const QuickbarService &InventoryController::GetQuickbarService() const { return m_quickbarSvc; }

// ----- Initialization -----

void InventoryController::InitializeDefaultCapacities() {
    m_capacitySvc.SetBaseCapacity(EInventoryBucket::ConsumableBucket, 2000);
    m_capacitySvc.SetBaseCapacity(EInventoryBucket::MaterialBucket, 2000);
    m_capacitySvc.SetBaseCapacity(EInventoryBucket::QuestBucket, 500);
    m_capacitySvc.SetBaseCapacity(EInventoryBucket::WeaponBucket, 2000);
    m_capacitySvc.SetBaseCapacity(EInventoryBucket::ArtifactBucket, 1500);
    m_capacitySvc.SetBaseCapacity(EInventoryBucket::GiftPackBucket, 500);
}

// ----- Core Commands -----

EAcquireResult InventoryController::PickupItem(int itemId, int count) { return m_commandSvc.PickupItem(itemId, count); }

EAcquireResult InventoryController::AcquireItem(int itemId, int count, EAcquireReason reason,
                                                EOverflowPolicy overflowPolicy) {
    return m_commandSvc.AcquireItem(itemId, count, reason, overflowPolicy);
}

EItemUseResult InventoryController::UseEntry(const EntryKey &key, int count) {
    return m_commandSvc.UseEntry(key, count);
}

EDeleteResult InventoryController::DeleteEntry(const EntryKey &key, int count) {
    return m_commandSvc.DeleteEntry(key, count);
}

// ----- Quickbar -----

bool InventoryController::BindQuickbarItem(int slotIndex, int itemId) {
    return m_quickbarSvc.BindItemId(slotIndex, itemId);
}

bool InventoryController::UnbindQuickbarItem(int slotIndex) { return m_quickbarSvc.Unbind(slotIndex); }

EItemUseResult InventoryController::UseQuickbarSlot(int slotIndex) { return m_quickbarSvc.UseSlot(slotIndex); }

const QuickbarState &InventoryController::GetQuickbarState() const { return m_quickbarSvc.GetState(); }

// ----- Query / View -----

std::vector<InventoryEntryView> InventoryController::BuildPageEntries(EInventoryPage page) const {
    return m_querySvc.BuildEntriesForPage(page);
}

std::vector<InventoryEntryView> InventoryController::GetPageEntries(EInventoryPage page, const InventoryFilter *filter,
                                                                    const InventorySortSpec *sortSpec,
                                                                    bool applyLayoutOrder, bool syncLayoutWhenMissing) {
    std::vector<InventoryEntryView> entries = m_querySvc.BuildEntriesForPage(page);

    if (filter != nullptr) { entries = m_querySvc.ApplyFilter(entries, *filter); }

    if (sortSpec != nullptr) { m_querySvc.SortEntries(entries, page, *sortSpec); }

    if (syncLayoutWhenMissing) {
        const InventoryLayoutState *layout = m_layoutSvc.GetLayout(page);
        if (layout == nullptr || layout->visualOrder.empty()) {
            m_layoutSvc.SetVisualOrder(page, ExtractEntryKeys(entries));
        }
    }

    if (applyLayoutOrder) { entries = ApplyLayoutOrder(page, entries); }

    return entries;
}

void InventoryController::SortPage(EInventoryPage page, const InventorySortSpec &sortSpec) {
    std::vector<InventoryEntryView> entries = m_querySvc.BuildEntriesForPage(page);
    m_querySvc.SortEntries(entries, page, sortSpec);
    m_layoutSvc.SetVisualOrder(page, ExtractEntryKeys(entries));
}

bool InventoryController::MoveEntryOnPage(EInventoryPage page, const EntryKey &dragged, int targetIndex) {
    return m_layoutSvc.MoveEntry(page, dragged, targetIndex);
}

bool InventoryController::RestoreManualLayout(EInventoryPage page) { return m_layoutSvc.RestoreManualLayout(page); }

// ----- Layout Helpers -----

std::vector<EntryKey> InventoryController::ExtractEntryKeys(const std::vector<InventoryEntryView> &entries) const {
    std::vector<EntryKey> keys;
    keys.reserve(entries.size());

    for (const InventoryEntryView &entry : entries) { keys.push_back(entry.key); }

    return keys;
}

std::vector<InventoryEntryView>
InventoryController::ApplyLayoutOrder(EInventoryPage page, const std::vector<InventoryEntryView> &entries) const {
    const InventoryLayoutState *layout = m_layoutSvc.GetLayout(page);
    if (layout == nullptr || layout->visualOrder.empty() || entries.empty()) { return entries; }

    std::unordered_map<EntryKey, InventoryEntryView, EntryKeyHash> entryMap;
    entryMap.reserve(entries.size());

    for (const InventoryEntryView &entry : entries) { entryMap.emplace(entry.key, entry); }

    std::vector<InventoryEntryView> ordered;
    ordered.reserve(entries.size());

    std::unordered_set<EntryKey, EntryKeyHash> used;
    used.reserve(entries.size());

    for (const EntryKey &key : layout->visualOrder) {
        auto it = entryMap.find(key);
        if (it != entryMap.end()) {
            ordered.push_back(it->second);
            used.insert(key);
        }
    }

    for (const InventoryEntryView &entry : entries) {
        if (used.find(entry.key) == used.end()) { ordered.push_back(entry); }
    }

    return ordered;
}

// ----- Capacity / Wallet -----

InventoryCapacityState InventoryController::GetCapacityState() const { return m_capacitySvc.BuildCapacityState(); }

BucketCapacityState InventoryController::GetBucketState(EInventoryBucket bucket) const {
    return m_capacitySvc.GetBucketState(bucket);
}

std::int64_t InventoryController::GetCurrencyBalance(ECurrencyType currencyType) const {
    return m_walletSvc.GetBalance(currencyType);
}

EInventoryPage InventoryController::BucketToPage(EInventoryBucket bucket) {
    switch (bucket) {
    case EInventoryBucket::ConsumableBucket:
        return EInventoryPage::Consumables;
    case EInventoryBucket::MaterialBucket:
        return EInventoryPage::Materials;
    case EInventoryBucket::QuestBucket:
        return EInventoryPage::QuestItems;
    case EInventoryBucket::WeaponBucket:
        return EInventoryPage::Weapons;
    case EInventoryBucket::ArtifactBucket:
        return EInventoryPage::Artifacts;
    case EInventoryBucket::GiftPackBucket:
        return EInventoryPage::GiftPacks;
    case EInventoryBucket::None:
        return EInventoryPage::Consumables;
    }
}