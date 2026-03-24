#include "QuickbarService.h"
#include <cstddef>

QuickbarService::QuickbarService(InventoryCommandService &commandSvc, InventoryRuleService &ruleSvc,
                                 InventoryRepository &repo)
    : m_commandSvc(commandSvc), m_ruleSvc(ruleSvc), m_repo(repo) {
    for (int i = 0; i < QuickbarState::kMaxSlots; ++i) {
        size_t si = static_cast<size_t>(i);
        m_state.slots[si].slotIndex = i;
        m_state.slots[si].bindType = EQuickbarBindType::None;
        m_state.slots[si].boundItemId = 0;
        m_state.slots[si].enabled = true;
    }
}

bool QuickbarService::BindItemId(int slotIndex, int itemId) {
    if (!IsValidSlotIndex(slotIndex)) { return false; }

    if (!m_ruleSvc.CanBindToQuickbar(itemId)) { return false; }

    m_state.slots[static_cast<size_t>(slotIndex)].bindType = EQuickbarBindType::ItemId;
    m_state.slots[static_cast<size_t>(slotIndex)].boundItemId = itemId;
    m_state.slots[static_cast<size_t>(slotIndex)].enabled = true;
    return true;
}

bool QuickbarService::Unbind(int slotIndex) {
    if (!IsValidSlotIndex(slotIndex)) { return false; }

    m_state.slots[static_cast<size_t>(slotIndex)].bindType = EQuickbarBindType::None;
    m_state.slots[static_cast<size_t>(slotIndex)].boundItemId = 0;
    m_state.slots[static_cast<size_t>(slotIndex)].enabled = true;
    return true;
}

EItemUseResult QuickbarService::UseSlot(int slotIndex) {
    if (!IsValidSlotIndex(slotIndex)) { return EItemUseResult::InvalidItem; }

    QuickbarSlot &slot = m_state.slots[static_cast<size_t>(slotIndex)];
    if (!slot.enabled) { return EItemUseResult::RestrictedByState; }

    if (slot.bindType != EQuickbarBindType::ItemId || slot.boundItemId <= 0) { return EItemUseResult::InvalidItem; }

    StackEntry *stack = FindUsableStackByItemId(slot.boundItemId);
    if (stack == nullptr) { return EItemUseResult::CountNotEnough; }

    // 根据找到的 stack 构建 key
    EntryKey key;
    key.entryType = EEntryType::StackEntry;
    key.id = stack->stackId;

    return m_commandSvc.UseEntry(key, 1);
}

const QuickbarState &QuickbarService::GetState() const { return m_state; }

bool QuickbarService::IsValidSlotIndex(int slotIndex) const {
    return slotIndex >= 0 && slotIndex < QuickbarState::kMaxSlots;
}

StackEntry *QuickbarService::FindUsableStackByItemId(int itemId) const {
    std::vector<StackEntry> *entries = m_repo.FindStackEntries(itemId);
    if (entries == nullptr) { return nullptr; }

    // 优先用 normal stack，再用 overflow stack
    for (StackEntry &entry : *entries) {
        if (!entry.isOverflow && entry.count > 0) { return &entry; }
    }

    for (StackEntry &entry : *entries) {
        if (entry.isOverflow && entry.count > 0) { return &entry; }
    }

    return nullptr;
}