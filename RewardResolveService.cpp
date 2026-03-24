#include "RewardResolveService.h"

#include <cstddef>
#include <random>

RewardResolveService::RewardResolveService(const ItemConfigService &configSvc) : m_configSvc(configSvc) {}

ResolvedRewardResult RewardResolveService::ResolveGiftPackRewards(int giftPackItemId) const {
    ResolvedRewardResult result;

    const GiftPackConfig *giftCfg = m_configSvc.GetGiftPackConfig(giftPackItemId);
    if (giftCfg == nullptr) { return result; }

    for (const RewardEntryConfig &reward : giftCfg->rewards) {
        switch (reward.type) {
        // 解算礼包中固定的货币奖励
        case ERewardEntryType::FixedCurrency: {
            if (reward.currencyAmount > 0) {
                result.currencies.push_back(ResolvedCurrencyReward{reward.currencyType, reward.currencyAmount});
            }
            break;
        }
        // 解算礼包中固定的物品奖励
        case ERewardEntryType::FixedItem: {
            const int count = RollInRange(reward.minCount, reward.maxCount);
            if (reward.itemId > 0 && count > 0) { result.items.push_back(ResolvedItemReward{reward.itemId, count}); }
            break;
        }
        // 解算礼包中的随即物品奖励
        case ERewardEntryType::RandomItemGroup: {
            for (int i = 0; i < reward.randomPickCount; ++i) {
                const int index = PickRandomCandidateByWeight(reward.candidates); // 奖励在池子中的下标
                if (index < 0 || index >= static_cast<int>(reward.candidates.size())) { continue; } // 无效奖励

                const RandomRewardCandidate &candidate = reward.candidates[static_cast<size_t>(index)];
                const int count = RollInRange(candidate.minCount, candidate.maxCount); // 发放的数量
                if (candidate.itemId > 0 && count > 0) {
                    result.items.push_back(ResolvedItemReward{candidate.itemId, count});
                }
            }
            break;
        }
        }
    }

    return result;
}

int RewardResolveService::RollInRange(int minValue, int maxValue) const {
    if (maxValue < minValue) { return minValue; }

    // thread local
    // 1 - 随机数引擎不是线程安全的，所以给每个线程分配一个
    // 2 - 每个线程都沿着自己的随机序列前进，不会和别的线程互相印象
    // static - 每个线程都初始化且仅初始化一次
    static thread_local std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(minValue, maxValue);
    return dist(rng);
}

int RewardResolveService::PickRandomCandidateByWeight(const std::vector<RandomRewardCandidate> &candidates) const {
    if (candidates.empty()) { return -1; }

    // 统计权重
    int totalWeight = 0;
    for (const RandomRewardCandidate &candidate : candidates) {
        if (candidate.weight > 0) { totalWeight += candidate.weight; }
    }

    if (totalWeight <= 0) { return -1; }

    static thread_local std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(1, totalWeight);
    int roll = dist(rng);

    // 计算最终奖励在池子中的下标
    int prefix = 0;
    for (size_t i = 0; i < candidates.size(); ++i) {
        if (candidates[i].weight <= 0) { continue; }

        prefix += candidates[i].weight;
        if (roll <= prefix) { return static_cast<int>(i); }
    }

    return -1;
}