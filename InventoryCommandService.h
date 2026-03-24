#pragma once

#include "InventoryCapacityService.h"
#include "InventoryRepository.h"
#include "InventoryRuleService.h"
#include "ItemConfigService.h"
#include "RewardResolveService.h"
#include "WalletService.h"

// 指令执行层
class InventoryCommandService {
public:
    InventoryCommandService(ItemConfigService &configSvc, InventoryRepository &repo, WalletService &walletSvc,
                            InventoryCapacityService &capacitySvc, InventoryRuleService &ruleSvc,
                            RewardResolveService &rewardSvc);

    // 拾取 count 个指定 id 的物品
    EAcquireResult PickupItem(int itemId, int count);

    // 获取 count 个指定 id 的物品 (邮件发放/兑换码/礼包开启)
    EAcquireResult AcquireItem(int itemId, int count, EAcquireReason reason, EOverflowPolicy overflowPolicy);

    // 获取一个实例物品
    EAcquireResult AcquireInstanceItem(const ItemInstance &instance, EAcquireReason reason,
                                       EOverflowPolicy overflowPolicy);

    // 使用指定 key 的格子中存放的物品
    EItemUseResult UseEntry(const EntryKey &key, int count = 1);
    // 删除指定 key 的格子中存放的物品
    EDeleteResult DeleteEntry(const EntryKey &key, int count = 1);

    // 开启一个存放在指定 key 的格子中的礼包
    EItemUseResult OpenGiftPack(const EntryKey &key);

private:
    // 获取 count 数量的指定 id 的货币
    EAcquireResult AcquireCurrencyItem(int itemId, int count);
    // 获取 count 数量的指定 id 的可堆叠物品
    EAcquireResult AcquireStackableItemInternal(int itemId, int count, EOverflowPolicy overflowPolicy);
    // 获取 count 数量的指定 id 的不可堆叠物品
    EAcquireResult AcquireNonStackableItemInternal(int itemId, int count, EOverflowPolicy overflowPolicy);

    // 在指定 id 的存放可堆叠物品的格子中消费 count 个物品
    bool ConsumeStackEntry(StackId stackId, int count);
    // 消费指定 id 的实例 (等价于直接删除此实例)
    bool ConsumeInstance(InstanceId instanceId);

    // 给予玩家已解算好的奖励
    bool TryApplyResolvedRewards(const ResolvedRewardResult &rewards);

    // 根据 id 返回货币类型
    std::optional<ECurrencyType> TryResolveCurrencyType(int itemId) const;

private:
    ItemConfigService &m_configSvc;
    InventoryRepository &m_repo;
    WalletService &m_walletSvc;
    InventoryCapacityService &m_capacitySvc;
    InventoryRuleService &m_ruleSvc;
    RewardResolveService &m_rewardSvc;
};