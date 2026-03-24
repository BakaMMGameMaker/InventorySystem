#pragma once

#include "InventoryRepository.h"
#include "ItemConfigService.h"

class InventoryQueryService {
public:
    InventoryQueryService(const ItemConfigService &configSvc, const InventoryRepository &repo);

    // 为每一页建立一个可视的格子列表
    std::vector<InventoryEntryView> BuildEntriesForPage(EInventoryPage page) const;

    // 在指定的格子列表上应用过滤器，返回过滤出来的格子
    std::vector<InventoryEntryView> ApplyFilter(const std::vector<InventoryEntryView> &entries,
                                                const InventoryFilter &filter) const;

    // 根据指定的排序键排序 entries 中的条目 (page 无用)
    void SortEntries(std::vector<InventoryEntryView> &entries, EInventoryPage page,
                     const InventorySortSpec &sortSpec) const;

private:
    // 创建存放可堆叠物品的格子的视图
    InventoryEntryView BuildViewFromStack(const StackEntry &stack, const ItemConfig &cfg) const;
    // 创建存放实例的格子的视图
    InventoryEntryView BuildViewFromInstance(const ItemInstance &instance, const ItemConfig &cfg) const;

    // cfg 代表的物品是否属于 page
    bool MatchesPage(const ItemConfig &cfg, EInventoryPage page) const;
    // 指定格子是否符合指定的过滤条件
    bool MatchesFilter(const InventoryEntryView &entry, const InventoryFilter &filter) const;

    // 两个格子用指定排序键比较的结果
    int CompareByOption(const InventoryEntryView &lhs, const InventoryEntryView &rhs, ESortOption option) const;

    // 是否升序排序
    bool IsAscending(ESortDirection dir) const;

private:
    const ItemConfigService &m_configSvc;
    const InventoryRepository &m_repo;
};