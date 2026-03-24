#pragma once

#include "InventoryTypes.h"

// 货币系统
class WalletService {
public:
    WalletService();

    std::int64_t GetBalance(ECurrencyType type) const;          // 获取给定类型货币的数量
    void AddCurrency(ECurrencyType type, std::int64_t amount);  // 增加给定类型货币指定的数量
    bool CostCurrency(ECurrencyType type, std::int64_t amount); // 扣除给定类型货币指定的数量

    const CurrencyWallet &GetWallet() const; // 货币仓库

private:
    CurrencyWallet m_wallet;
};