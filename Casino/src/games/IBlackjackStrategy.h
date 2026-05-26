#pragma once
#include <string>

class IBlackjackStrategy
{
public:
    virtual ~IBlackjackStrategy() = default;
    // Returns true if the dealer should draw another card
    virtual bool shouldDealerHit(int handValue, bool isSoft) const = 0;
    virtual std::string getName() const = 0;
};
