#include "InventoryQueryService.h"

#include <algorithm>
#include <cctype>

namespace {
// 给定字符串的全小写版本
std::string ToLowerCopy(std::string s) {
    for (char &ch : s) { ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch))); }
    return s;
}

// text 中是否包含 keyword (大小写不敏感)
bool ContainsCaseInsensitive(const std::string &text, const std::string &keyword) {
    if (keyword.empty()) { return true; }

    const std::string lowerText = ToLowerCopy(text);
    const std::string lowerKeyword = ToLowerCopy(keyword);
    return lowerText.find(lowerKeyword) != std::string::npos;
}
} // namespace

InventoryQueryService::InventoryQueryService(const ItemConfigService &configSvc, const InventoryRepository &repo)
    : m_configSvc(configSvc), m_repo(repo) {}

std::vector<InventoryEntryView> InventoryQueryService::BuildEntriesForPage(EInventoryPage page) const {
    std::vector<InventoryEntryView> result;

    for (const auto &[itemId, entries] : m_repo.GetAllStackEntries()) {
        const ItemConfig *cfg = m_configSvc.GetItemConfig(itemId);
        if (cfg == nullptr || !cfg->showInInventory) { continue; } // 跳过不显示的格子

        if (!MatchesPage(*cfg, page)) { continue; } // 跳过不属于当前页的格子

        for (const StackEntry &entry : entries) { result.push_back(BuildViewFromStack(entry, *cfg)); }
    }

    for (const auto &[instanceId, instance] : m_repo.GetAllInstances()) {
        (void)instanceId;

        const ItemConfig *cfg = m_configSvc.GetItemConfig(instance.itemId);
        if (cfg == nullptr || !cfg->showInInventory) { continue; }

        if (!MatchesPage(*cfg, page)) { continue; }

        result.push_back(BuildViewFromInstance(instance, *cfg));
    }

    return result;
}

std::vector<InventoryEntryView> InventoryQueryService::ApplyFilter(const std::vector<InventoryEntryView> &entries,
                                                                   const InventoryFilter &filter) const {
    std::vector<InventoryEntryView> result;
    result.reserve(entries.size());

    for (const InventoryEntryView &entry : entries) {
        if (MatchesFilter(entry, filter)) { result.push_back(entry); }
    }

    return result;
}

void InventoryQueryService::SortEntries(std::vector<InventoryEntryView> &entries, EInventoryPage page,
                                        const InventorySortSpec &sortSpec) const {
    (void)page;

    std::stable_sort(entries.begin(), entries.end(),
                     [this, &sortSpec](const InventoryEntryView &lhs, const InventoryEntryView &rhs) {
                         const int primaryCmp = CompareByOption(lhs, rhs, sortSpec.primary);
                         if (primaryCmp != 0) {
                             return IsAscending(sortSpec.primaryDir) ? (primaryCmp < 0) : (primaryCmp > 0);
                         }

                         const int secondaryCmp = CompareByOption(lhs, rhs, sortSpec.secondary);
                         if (secondaryCmp != 0) {
                             return IsAscending(sortSpec.secondaryDir) ? (secondaryCmp < 0) : (secondaryCmp > 0);
                         }

                         return lhs.key.id < rhs.key.id;
                     });
}

InventoryEntryView InventoryQueryService::BuildViewFromStack(const StackEntry &stack, const ItemConfig &cfg) const {
    InventoryEntryView view;
    view.key = EntryKey{EEntryType::StackEntry, stack.stackId};

    view.itemId = cfg.itemId;
    view.displayName = cfg.name;
    view.mainCategory = cfg.mainCategory;
    view.subCategory = cfg.subCategory;
    view.rarity = cfg.rarity;

    view.displayCount = stack.count;
    view.level = 0;

    view.isOverflow = stack.isOverflow;
    view.isNew = stack.isNew;
    view.isLocked = false;
    view.isEquipped = false;
    view.equippedCharacterId = 0;

    view.filterTags = cfg.tags;
    view.sortTypeKey = static_cast<int>(cfg.subCategory);
    view.sortTimeKey = stack.createTime;
    return view;
}

InventoryEntryView InventoryQueryService::BuildViewFromInstance(const ItemInstance &instance,
                                                                const ItemConfig &cfg) const {
    InventoryEntryView view;
    view.key = EntryKey{EEntryType::InstanceEntry, instance.instanceId};

    view.itemId = cfg.itemId;
    view.displayName = cfg.name;
    view.mainCategory = cfg.mainCategory;
    view.subCategory = cfg.subCategory;
    view.rarity = cfg.rarity;

    view.displayCount = 1;
    view.level = instance.level;

    view.isOverflow = instance.isOverflow;
    view.isNew = instance.isNew;
    view.isLocked = instance.isLocked;
    view.isEquipped = instance.isEquipped;
    view.equippedCharacterId = instance.equippedCharacterId;

    view.filterTags = cfg.tags;
    view.sortTypeKey = static_cast<int>(cfg.subCategory);
    view.sortTimeKey = instance.createTime;
    return view;
}

bool InventoryQueryService::MatchesPage(const ItemConfig &cfg, EInventoryPage page) const {
    switch (page) {
    case EInventoryPage::Consumables:
        return cfg.mainCategory == EItemMainCategory::Consumable;
    case EInventoryPage::Materials:
        return cfg.mainCategory == EItemMainCategory::Material;
    case EInventoryPage::QuestItems:
        return cfg.mainCategory == EItemMainCategory::QuestItem;
    case EInventoryPage::Weapons:
        return cfg.mainCategory == EItemMainCategory::Weapon;
    case EInventoryPage::Artifacts:
        return cfg.mainCategory == EItemMainCategory::Artifact;
    case EInventoryPage::GiftPacks:
        return cfg.mainCategory == EItemMainCategory::GiftPack;
    }
}

bool InventoryQueryService::MatchesFilter(const InventoryEntryView &entry, const InventoryFilter &filter) const {
    if (filter.mainCategory.has_value() && entry.mainCategory != *filter.mainCategory) { return false; }

    if (filter.subCategory.has_value() && entry.subCategory != *filter.subCategory) { return false; }

    if (!filter.rarities.empty() && filter.rarities.find(entry.rarity) == filter.rarities.end()) { return false; }

    if (!filter.subCategories.empty() && filter.subCategories.find(entry.subCategory) == filter.subCategories.end()) {
        return false;
    }

    if (filter.minLevel.has_value() && entry.level < *filter.minLevel) { return false; }

    if (filter.maxLevel.has_value() && entry.level > *filter.maxLevel) { return false; }

    if (filter.onlyLocked.has_value() && *filter.onlyLocked && !entry.isLocked) { return false; }

    if (filter.onlyUnlocked.has_value() && *filter.onlyUnlocked && entry.isLocked) { return false; }

    if (filter.onlyEquipped.has_value() && *filter.onlyEquipped && !entry.isEquipped) { return false; }

    if (filter.onlyUnequipped.has_value() && *filter.onlyUnequipped && entry.isEquipped) { return false; }

    if (filter.onlyOverflow.has_value() && *filter.onlyOverflow && !entry.isOverflow) { return false; }

    if (!filter.keyword.empty()) {
        // 根据条目名称匹配关键词
        bool matched = ContainsCaseInsensitive(entry.displayName, filter.keyword);
        if (!matched) {
            // 根据条目的标签匹配关键词
            for (const std::string &tag : entry.filterTags) {
                if (ContainsCaseInsensitive(tag, filter.keyword)) {
                    matched = true;
                    break;
                }
            }
        }

        if (!matched) { return false; }
    }

    return true;
}

int InventoryQueryService::CompareByOption(const InventoryEntryView &lhs, const InventoryEntryView &rhs,
                                           ESortOption option) const {
    switch (option) {
    case ESortOption::Default:
        // 默认排序规则
        // 1 - 稀有度
        // 2 - 子类别
        // 3 - 等级
        if (lhs.rarity != rhs.rarity) { return static_cast<int>(lhs.rarity) - static_cast<int>(rhs.rarity); }
        if (lhs.sortTypeKey != rhs.sortTypeKey) { return lhs.sortTypeKey - rhs.sortTypeKey; }
        if (lhs.level != rhs.level) { return lhs.level - rhs.level; }
        return 0;

    case ESortOption::Rarity:
        return static_cast<int>(lhs.rarity) - static_cast<int>(rhs.rarity);

    case ESortOption::Level:
        return lhs.level - rhs.level;

    case ESortOption::Type:
        return lhs.sortTypeKey - rhs.sortTypeKey;

    case ESortOption::AcquireTime:
        if (lhs.sortTimeKey < rhs.sortTimeKey) return -1;
        if (lhs.sortTimeKey > rhs.sortTimeKey) return 1;
        return 0;

    case ESortOption::Name:
        if (lhs.displayName < rhs.displayName) return -1;
        if (lhs.displayName > rhs.displayName) return 1;
        return 0;

    case ESortOption::EquippedState:
        return static_cast<int>(lhs.isEquipped) - static_cast<int>(rhs.isEquipped);

    case ESortOption::LockedState:
        return static_cast<int>(lhs.isLocked) - static_cast<int>(rhs.isLocked);
    }
}

bool InventoryQueryService::IsAscending(ESortDirection dir) const { return dir == ESortDirection::Asc; }