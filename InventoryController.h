#pragma once

#include "InventoryCapacityService.h"
#include "InventoryCommandService.h"
#include "InventoryLayoutService.h"
#include "InventoryQueryService.h"
#include "InventoryRepository.h"
#include "InventoryRuleService.h"
#include "ItemConfigService.h"
#include "QuickbarService.h"
#include "RewardResolveService.h"
#include "WalletService.h"


class InventoryController {
public:
    InventoryController();

    // ----- Accessors -----
    ItemConfigService &GetConfigService();
    InventoryRepository &GetRepository();
    WalletService &GetWalletService();
    InventoryCapacityService &GetCapacityService();
    InventoryRuleService &GetRuleService();
    InventoryCommandService &GetCommandService();
    InventoryQueryService &GetQueryService();
    InventoryLayoutService &GetLayoutService();
    QuickbarService &GetQuickbarService();

    const ItemConfigService &GetConfigService() const;
    const InventoryRepository &GetRepository() const;
    const WalletService &GetWalletService() const;
    const InventoryCapacityService &GetCapacityService() const;
    const InventoryRuleService &GetRuleService() const;
    const InventoryCommandService &GetCommandService() const;
    const InventoryQueryService &GetQueryService() const;
    const InventoryLayoutService &GetLayoutService() const;
    const QuickbarService &GetQuickbarService() const;

    // ----- Initialization -----
    void InitializeDefaultCapacities();

    // ----- Core Commands -----
    EAcquireResult PickupItem(int itemId, int count);
    EAcquireResult AcquireItem(int itemId, int count, EAcquireReason reason, EOverflowPolicy overflowPolicy);
    EItemUseResult UseEntry(const EntryKey &key, int count = 1);
    EDeleteResult DeleteEntry(const EntryKey &key, int count = 1);

    // ----- Quickbar -----
    bool BindQuickbarItem(int slotIndex, int itemId);
    bool UnbindQuickbarItem(int slotIndex);
    EItemUseResult UseQuickbarSlot(int slotIndex);
    const QuickbarState &GetQuickbarState() const;

    // ----- Query / View -----
    std::vector<InventoryEntryView> BuildPageEntries(EInventoryPage page) const;

    std::vector<InventoryEntryView> GetPageEntries(EInventoryPage page, const InventoryFilter *filter,
                                                   const InventorySortSpec *sortSpec, bool applyLayoutOrder,
                                                   bool syncLayoutWhenMissing);

    void SortPage(EInventoryPage page, const InventorySortSpec &sortSpec);

    bool MoveEntryOnPage(EInventoryPage page, const EntryKey &dragged, int targetIndex);
    bool RestoreManualLayout(EInventoryPage page);

    // ----- Layout Helpers -----
    std::vector<EntryKey> ExtractEntryKeys(const std::vector<InventoryEntryView> &entries) const;
    std::vector<InventoryEntryView> ApplyLayoutOrder(EInventoryPage page,
                                                     const std::vector<InventoryEntryView> &entries) const;

    // ----- Capacity / Wallet -----
    InventoryCapacityState GetCapacityState() const;
    BucketCapacityState GetBucketState(EInventoryBucket bucket) const;
    std::int64_t GetCurrencyBalance(ECurrencyType currencyType) const;

private:
    static EInventoryPage BucketToPage(EInventoryBucket bucket);

private:
    ItemConfigService m_configSvc;
    InventoryRepository m_repo;
    WalletService m_walletSvc;
    InventoryCapacityService m_capacitySvc;
    InventoryRuleService m_ruleSvc;
    RewardResolveService m_rewardSvc;
    InventoryCommandService m_commandSvc;
    InventoryQueryService m_querySvc;
    InventoryLayoutService m_layoutSvc;
    QuickbarService m_quickbarSvc;
};