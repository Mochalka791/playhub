#include "DealerHitSoft17.h"

bool DealerHitSoft17::shouldDealerHit(int handValue, bool isSoft) const
{
    // Dealer must hit on 16 or less, AND on soft 17
    if (handValue < 17) return true;
    if (handValue == 17 && isSoft) return true;
    return false;
}
