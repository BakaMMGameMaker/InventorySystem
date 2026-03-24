#include "WalletService.h"

WalletService::WalletService() {
    m_wallet.balances[ECurrencyType::Mora] = 0;
    m_wallet.balances[ECurrencyType::Primogem] = 0;
}

std::int64_t WalletService::GetBalance(ECurrencyType type) const {
    auto it = m_wallet.balances.find(type);
    if (it == m_wallet.balances.end()) { return 0; }
    return it->second;
}

void WalletService::AddCurrency(ECurrencyType type, std::int64_t amount) {
    if (amount <= 0) { return; }

    m_wallet.balances[type] += amount;
}

bool WalletService::CostCurrency(ECurrencyType type, std::int64_t amount) {
    if (amount <= 0) { return false; }

    auto it = m_wallet.balances.find(type);
    if (it == m_wallet.balances.end()) { return false; }

    // 货币数量不足返回 false
    if (it->second < amount) { return false; }

    it->second -= amount;
    return true;
}

const CurrencyWallet &WalletService::GetWallet() const { return m_wallet; }