#include "SlotMachine.h"
#include "../models/Player.h"
#include "../core/EventSystem.h"
#include <random>
#include <stdexcept>

SlotMachine::SlotMachine(Player& player) : player(player) {}

void SlotMachine::placeBet(double amount)
{
    if (!player.canBet(amount))
        throw std::runtime_error("Insufficient balance");
    bet = amount;
}

void SlotMachine::play()
{
    if (bet <= 0.0)
        throw std::runtime_error("Place a bet first");

    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<int> dist(0, static_cast<int>(SlotSymbol::COUNT) - 1);

    for (auto& r : reels)
        r = static_cast<SlotSymbol>(dist(rng));

    calculateResult();

    if (result == GameResult::Win) {
        player.addWin(payout - bet);
        EventBus::instance().publish(WinEvent{player.getName(), payout, getName()});
    } else if (result == GameResult::Push) {
        // no balance change on push
        EventBus::instance().publish(GameEndEvent{player.getName(), getName(), 0.0});
        return;
    } else {
        player.addLoss(bet);
        EventBus::instance().publish(LossEvent{player.getName(), bet, getName()});
    }

    EventBus::instance().publish(GameEndEvent{
        player.getName(), getName(), (result == GameResult::Win) ? payout - bet : -bet
    });
}

void SlotMachine::calculateResult()
{
    bool r01 = (reels[0] == reels[1]);
    bool r12 = (reels[1] == reels[2]);
    bool all  = r01 && r12;

    if (!all && !r01 && !r12) {
        payout = 0.0;
        result = GameResult::Loss;
        return;
    }

    if (r01 || r12) {
        if (!all) {
            payout = bet * 1.0;
            result = GameResult::Push;
            return;
        }
    }

    // All three match — look up multiplier
    double mult = 1.0;
    switch (reels[0]) {
        case SlotSymbol::Crown:   mult = 20.0; break;
        case SlotSymbol::Seven:   mult = 12.0; break;
        case SlotSymbol::Diamond: mult = 10.0; break;
        case SlotSymbol::Bar:     mult =  6.0; break;
        case SlotSymbol::Bell:    mult =  5.0; break;
        case SlotSymbol::Star:    mult =  4.0; break;
        case SlotSymbol::Cherry:  mult =  4.0; break;
        case SlotSymbol::Lemon:   mult =  3.0; break;
        case SlotSymbol::Orange:  mult =  3.0; break;
        case SlotSymbol::Clover:  mult =  2.0; break;
        default:                  mult =  2.0; break;
    }
    payout = bet * mult;
    result = GameResult::Win;
}

std::string SlotMachine::symbolToString(SlotSymbol s)
{
    switch (s) {
        case SlotSymbol::Crown:   return "CROWN";
        case SlotSymbol::Seven:   return "  7  ";
        case SlotSymbol::Diamond: return " DIA ";
        case SlotSymbol::Bar:     return " BAR ";
        case SlotSymbol::Bell:    return " BEL ";
        case SlotSymbol::Star:    return " STR ";
        case SlotSymbol::Cherry:  return "CHRRY";
        case SlotSymbol::Lemon:   return "LEMN ";
        case SlotSymbol::Orange:  return "ORNG ";
        case SlotSymbol::Clover:  return "CLVR ";
        default:                  return "  ?  ";
    }
}

double SlotMachine::symbolMultiplier(SlotSymbol s)
{
    switch (s) {
        case SlotSymbol::Crown:   return 20.0;
        case SlotSymbol::Seven:   return 12.0;
        case SlotSymbol::Diamond: return 10.0;
        case SlotSymbol::Bar:     return  6.0;
        case SlotSymbol::Bell:    return  5.0;
        case SlotSymbol::Star:    return  4.0;
        case SlotSymbol::Cherry:  return  4.0;
        case SlotSymbol::Lemon:   return  3.0;
        case SlotSymbol::Orange:  return  3.0;
        case SlotSymbol::Clover:  return  2.0;
        default:                  return  1.0;
    }
}
