#pragma once
#include "IBlackjackStrategy.h"

// Alternative rule: dealer stands on soft 17
class DealerStandSoft17 : public IBlackjackStrategy
{
public:
    bool shouldDealerHit(int handValue, bool isSoft) const override;
    std::string getName() const override { return "Stand Soft 17"; }
};
