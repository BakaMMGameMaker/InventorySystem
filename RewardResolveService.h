#pragma once

#include "ItemConfigService.h"

class RewardResolveService {
public:
    explicit RewardResolveService(const ItemConfigService &configSvc);

    // 解算指定 id 的礼包奖励
    ResolvedRewardResult ResolveGiftPackRewards(int giftPackItemId) const;

private:
    // 返回指定区间内的随机数
    int RollInRange(int minValue, int maxValue) const;
    // 在奖励池中根据权重随机选择一个物品，并返回其下标
    int PickRandomCandidateByWeight(const std::vector<RandomRewardCandidate> &candidates) const;

private:
    const ItemConfigService &m_configSvc;
};