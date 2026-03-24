#pragma once

#include "InventoryTypes.h"

// 物品配置
class ItemConfigService {
public:
    ItemConfigService() = default;

    // 获取指定 id 的物品配置
    const ItemConfig *GetItemConfig(int itemId) const;
    // 获取指定 id 的礼包配置
    const GiftPackConfig *GetGiftPackConfig(int itemId) const;

    // 指定 id 的物品是否为礼包
    bool IsGiftPack(int itemId) const;
    // 指定 id 的物品是否为货币
    bool IsCurrencyItem(int itemId) const;

    // 新增一个配置
    void AddItemConfig(const ItemConfig &config);
    // 新增一个礼包配置
    void AddGiftPackConfig(const GiftPackConfig &config);

private:
    std::unordered_map<int, ItemConfig> m_itemConfigs;         // id => 物品
    std::unordered_map<int, GiftPackConfig> m_giftPackConfigs; // id => 礼包
};