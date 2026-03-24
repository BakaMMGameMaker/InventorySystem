#pragma once

#include "InventoryTypes.h"

class InventoryLayoutService {
public:
    // 获取指定页的页面布局状态
    const InventoryLayoutState *GetLayout(EInventoryPage page) const;

    // 获取可修改的页面布局状态
    InventoryLayoutState *GetMutableLayout(EInventoryPage page);

    // 将指定页面的视觉布局设置为指定的 order
    void SetVisualOrder(EInventoryPage page, const std::vector<EntryKey> &order);

    // 拖拽指定 key 的格子到目标位置
    bool MoveEntry(EInventoryPage page, const EntryKey &dragged, int targetIndex);

    // 将当前页面的布局保存到快照中
    void SaveManualSnapshot(EInventoryPage page);
    // 从快照中恢复保存的布局
    bool RestoreManualLayout(EInventoryPage page);

private:
    // 确保指定页存在，并返回相应的页面布局状态
    InventoryLayoutState &EnsureLayout(EInventoryPage page);

private:
    std::unordered_map<EInventoryPage, InventoryLayoutState> m_layouts; // 页面 Enum => 页面布局状态
};