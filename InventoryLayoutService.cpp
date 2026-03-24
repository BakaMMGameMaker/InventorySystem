#include "InventoryLayoutService.h"

#include <algorithm>

const InventoryLayoutState *InventoryLayoutService::GetLayout(EInventoryPage page) const {
    auto it = m_layouts.find(page);
    if (it == m_layouts.end()) { return nullptr; }
    return &it->second;
}

InventoryLayoutState *InventoryLayoutService::GetMutableLayout(EInventoryPage page) { return &EnsureLayout(page); }

void InventoryLayoutService::SetVisualOrder(EInventoryPage page, const std::vector<EntryKey> &order) {
    InventoryLayoutState &layout = EnsureLayout(page);
    layout.visualOrder = order;
}

bool InventoryLayoutService::MoveEntry(EInventoryPage page, const EntryKey &dragged, int targetIndex) {
    InventoryLayoutState &layout = EnsureLayout(page);
    if (layout.visualOrder.empty()) { return false; } // 布局为空

    auto it = std::find(layout.visualOrder.begin(), layout.visualOrder.end(), dragged);
    if (it == layout.visualOrder.end()) { return false; } // 布局中不存在指定条目

    const int oldIndex = static_cast<int>(std::distance(layout.visualOrder.begin(), it));
    if (oldIndex == targetIndex) { return true; }

    if (targetIndex < 0) { targetIndex = 0; }
    if (targetIndex >= static_cast<int>(layout.visualOrder.size())) {
        targetIndex = static_cast<int>(layout.visualOrder.size()) - 1;
    }

    EntryKey moved = *it;
    layout.visualOrder.erase(it);

    if (targetIndex > oldIndex) { --targetIndex; } // 因为 oldindex 后面的条目都左移了一格

    layout.visualOrder.insert(layout.visualOrder.begin() + targetIndex, moved);

    layout.manualSnapshot = layout.visualOrder; // 保存玩家的自定义布局快照
    layout.hasCustomLayout = true;              // 将 ‘有自定义布局’ 置位
    return true;
}

void InventoryLayoutService::SaveManualSnapshot(EInventoryPage page) {
    InventoryLayoutState &layout = EnsureLayout(page);
    layout.manualSnapshot = layout.visualOrder;
    layout.hasCustomLayout = true;
}

bool InventoryLayoutService::RestoreManualLayout(EInventoryPage page) {
    InventoryLayoutState &layout = EnsureLayout(page);
    if (!layout.hasCustomLayout || layout.manualSnapshot.empty()) { return false; }

    layout.visualOrder = layout.manualSnapshot;
    return true;
}

InventoryLayoutState &InventoryLayoutService::EnsureLayout(EInventoryPage page) {
    auto [it, inserted] = m_layouts.try_emplace(page);
    (void)inserted;

    it->second.page = page;
    return it->second;
}