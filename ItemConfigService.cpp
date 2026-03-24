#include "ItemConfigService.h"

const ItemConfig *ItemConfigService::GetItemConfig(int itemId) const {
    auto it = m_itemConfigs.find(itemId);
    if (it == m_itemConfigs.end()) { return nullptr; }
    return &it->second;
}

const GiftPackConfig *ItemConfigService::GetGiftPackConfig(int itemId) const {
    auto it = m_giftPackConfigs.find(itemId);
    if (it == m_giftPackConfigs.end()) { return nullptr; }
    return &it->second;
}

bool ItemConfigService::IsGiftPack(int itemId) const {
    const ItemConfig *cfg = GetItemConfig(itemId);
    if (cfg == nullptr) { return false; }

    return cfg->mainCategory == EItemMainCategory::GiftPack;
}

bool ItemConfigService::IsCurrencyItem(int itemId) const {
    const ItemConfig *cfg = GetItemConfig(itemId);
    if (cfg == nullptr) { return false; }

    return cfg->mainCategory == EItemMainCategory::Currency;
}

void ItemConfigService::AddItemConfig(const ItemConfig &config) { m_itemConfigs[config.itemId] = config; }

void ItemConfigService::AddGiftPackConfig(const GiftPackConfig &config) { m_giftPackConfigs[config.itemId] = config; }