#pragma once

#include "InventoryTypes.h"

// 物品配置
class ItemConfigService {
public:
    ItemConfigService() = default;

    const ItemConfig *GetItemConfig(int itemId) const;         // 获取物品配置
    const GiftPackConfig *GetGiftPackConfig(int itemId) const; // 获取礼包配置

    bool IsGiftPack(int itemId) const;     // 是否礼包
    bool IsCurrencyItem(int itemId) const; // 是否货币

    void AddItemConfig(const ItemConfig &config);         // 增加配置
    void AddGiftPackConfig(const GiftPackConfig &config); // 增加礼包配置

private:
    std::unordered_map<int, ItemConfig> m_itemConfigs;         // id => 物品
    std::unordered_map<int, GiftPackConfig> m_giftPackConfigs; // id => 礼包
};