#pragma once
#include "IBlackjackStrategy.h"

// Standard casino rule: dealer hits on soft 17
class DealerHitSoft17 : public IBlackjackStrategy
{
public:
    bool shouldDealerHit(int handValue, bool isSoft) const override;
    std::string getName() const override { return "Hit Soft 17"; }
};
