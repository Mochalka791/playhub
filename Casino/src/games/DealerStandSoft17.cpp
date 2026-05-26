#include "DealerStandSoft17.h"

bool DealerStandSoft17::shouldDealerHit(int handValue, bool /*isSoft*/) const
{
    // Dealer always stands on 17 or higher
    return handValue < 17;
}
