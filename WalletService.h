#pragma once

#include "InventoryTypes.h"

// 货币系统
class WalletService {
public:
    WalletService();

    // 指定类型货币的数量
    std::int64_t GetBalance(ECurrencyType type) const;
    // 增加指定类型货币指定的数量
    void AddCurrency(ECurrencyType type, std::int64_t amount);
    // 扣除指定类型货币指定的数量
    bool CostCurrency(ECurrencyType type, std::int64_t amount);

    // 获取货币仓库
    const CurrencyWallet &GetWallet() const;

private:
    CurrencyWallet m_wallet;
};