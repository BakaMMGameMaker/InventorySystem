#include "InventoryRepository.h"

const std::unordered_map<int, std::vector<StackEntry>> &InventoryRepository::GetAllStackEntries() const {
    return m_stackEntriesByItemId;
}

std::vector<StackEntry> *InventoryRepository::FindStackEntries(int itemId) {
    auto it = m_stackEntriesByItemId.find(itemId);
    if (it == m_stackEntriesByItemId.end()) { return nullptr; }
    return &it->second;
}

const std::vector<StackEntry> *InventoryRepository::FindStackEntries(int itemId) const {
    auto it = m_stackEntriesByItemId.find(itemId);
    if (it == m_stackEntriesByItemId.end()) { return nullptr; }
    return &it->second;
}

StackEntry *InventoryRepository::FindStackById(StackId stackId) {
    for (auto &[itemId, entries] : m_stackEntriesByItemId) {
        (void)itemId;
        for (auto &entry : entries) {
            if (entry.stackId == stackId) { return &entry; }
        }
    }
    return nullptr;
}

const StackEntry *InventoryRepository::FindStackById(StackId stackId) const {
    for (const auto &[itemId, entries] : m_stackEntriesByItemId) {
        (void)itemId;
        for (const auto &entry : entries) {
            if (entry.stackId == stackId) { return &entry; }
        }
    }
    return nullptr;
}

void InventoryRepository::AddStackEntry(const StackEntry &entry) {
    m_stackEntriesByItemId[entry.itemId].push_back(entry);
}

bool InventoryRepository::RemoveStackEntry(StackId stackId) {
    for (auto itMap = m_stackEntriesByItemId.begin(); itMap != m_stackEntriesByItemId.end(); ++itMap) {
        auto &entries = itMap->second;
        auto it = std::find_if(entries.begin(), entries.end(),
                               [stackId](const StackEntry &entry) { return entry.stackId == stackId; });

        if (it != entries.end()) {
            entries.erase(it);
            if (entries.empty()) { m_stackEntriesByItemId.erase(itMap); }
            return true;
        }
    }
    return false;
}

bool InventoryRepository::UpdateStackCount(StackId stackId, int newCount) {
    StackEntry *entry = FindStackById(stackId);
    if (entry == nullptr) { return false; }

    entry->count = newCount;
    return true;
}

bool InventoryRepository::SetStackOverflow(StackId stackId, bool isOverflow) {
    StackEntry *entry = FindStackById(stackId);
    if (entry == nullptr) { return false; }

    entry->isOverflow = isOverflow;
    return true;
}

const std::unordered_map<InstanceId, ItemInstance> &InventoryRepository::GetAllInstances() const {
    return m_instancesById;
}

ItemInstance *InventoryRepository::FindInstance(InstanceId instanceId) {
    auto it = m_instancesById.find(instanceId);
    if (it == m_instancesById.end()) { return nullptr; }
    return &it->second;
}

const ItemInstance *InventoryRepository::FindInstance(InstanceId instanceId) const {
    auto it = m_instancesById.find(instanceId);
    if (it == m_instancesById.end()) { return nullptr; }
    return &it->second;
}

void InventoryRepository::AddInstance(const ItemInstance &instance) { m_instancesById[instance.instanceId] = instance; }

bool InventoryRepository::RemoveInstance(InstanceId instanceId) { return m_instancesById.erase(instanceId) > 0; }

bool InventoryRepository::SetInstanceOverflow(InstanceId instanceId, bool isOverflow) {
    ItemInstance *instance = FindInstance(instanceId);
    if (instance == nullptr) { return false; }

    instance->isOverflow = isOverflow;
    return true;
}

StackId InventoryRepository::GenerateNextStackId() { return m_nextStackId++; }

InstanceId InventoryRepository::GenerateNextInstanceId() { return m_nextInstanceId++; }