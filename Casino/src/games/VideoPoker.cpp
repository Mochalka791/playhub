#include "VideoPoker.h"
#include "../models/Player.h"
#include "../core/EventSystem.h"
#include <algorithm>
#include <stdexcept>
#include <map>

VideoPoker::VideoPoker(Player& p) : player(p) {}

void VideoPoker::placeBet(double amount)
{
    if (!player.canBet(amount))
        throw std::runtime_error("Insufficient balance");
    bet = amount;
}

void VideoPoker::play()
{
    if (bet <= 0.0) throw std::runtime_error("Place a bet first");
    deck = Deck{};
    deck.shuffle();
    holds.fill(false);
    for (int i = 0; i < 5; ++i) hand[i] = deck.draw();
    phase = Phase::Holding;
    result = GameResult::Loss;
    payout = 0.0;
    handRank = PokerRank::None;
    player.addLoss(bet); // deduct upfront; addWin restores on win
}

void VideoPoker::setHold(int idx, bool h)
{
    if (idx >= 0 && idx < 5) holds[idx] = h;
}

void VideoPoker::draw()
{
    if (phase != Phase::Holding) return;
    for (int i = 0; i < 5; ++i)
        if (!holds[i]) hand[i] = deck.draw();

    handRank = evaluate(hand);
    double mult = multiplier(handRank);
    phase = Phase::GameOver;

    if (mult > 0.0) {
        payout = bet * mult;
        result = GameResult::Win;
        player.addWin(bet + (payout - bet)); // restore + profit
        EventBus::instance().publish(WinEvent{player.getName(), payout - bet, getName()});
    } else {
        payout = 0.0;
        result = GameResult::Loss;
        EventBus::instance().publish(LossEvent{player.getName(), bet, getName()});
    }
    EventBus::instance().publish(GameEndEvent{player.getName(), getName(),
        result == GameResult::Win ? payout - bet : -bet});
}

std::string VideoPoker::rankName(PokerRank r)
{
    switch (r) {
    case PokerRank::JacksOrBetter: return "Jacks or Better";
    case PokerRank::TwoPair:       return "Two Pair";
    case PokerRank::ThreeOfAKind:  return "Three of a Kind";
    case PokerRank::Straight:      return "Straight";
    case PokerRank::Flush:         return "Flush";
    case PokerRank::FullHouse:     return "Full House";
    case PokerRank::FourOfAKind:   return "Four of a Kind";
    case PokerRank::StraightFlush: return "Straight Flush";
    case PokerRank::RoyalFlush:    return "Royal Flush";
    default:                       return "No Win";
    }
}

double VideoPoker::multiplier(PokerRank r)
{
    switch (r) {
    case PokerRank::JacksOrBetter: return 1.0;
    case PokerRank::TwoPair:       return 2.0;
    case PokerRank::ThreeOfAKind:  return 3.0;
    case PokerRank::Straight:      return 4.0;
    case PokerRank::Flush:         return 6.0;
    case PokerRank::FullHouse:     return 9.0;
    case PokerRank::FourOfAKind:   return 25.0;
    case PokerRank::StraightFlush: return 50.0;
    case PokerRank::RoyalFlush:    return 800.0;
    default:                       return 0.0;
    }
}

// ---- Evaluator --------------------------------------------------

static int rankVal(Rank r) { return static_cast<int>(r); }

PokerRank VideoPoker::evaluate(const std::array<Card, 5>& h)
{
    // Count rank frequencies
    std::map<int, int> freq;
    for (const auto& c : h) freq[rankVal(c.rank)]++;

    std::vector<int> counts;
    for (auto& kv : freq) counts.push_back(kv.second);
    std::sort(counts.rbegin(), counts.rend());

    // Flush?
    bool flush = true;
    for (int i = 1; i < 5; ++i)
        if (h[i].suit != h[0].suit) { flush = false; break; }

    // Straight?
    std::vector<int> vals;
    for (const auto& c : h) vals.push_back(rankVal(c.rank));
    std::sort(vals.begin(), vals.end());
    bool straight = (vals[4] - vals[0] == 4 && (int)freq.size() == 5);
    // Ace-low straight: A-2-3-4-5 (Ace=14, vals would be 2,3,4,5,14)
    if (!straight && vals[4] == static_cast<int>(Rank::Ace)) {
        std::vector<int> lowVals = {1, vals[0], vals[1], vals[2], vals[3]};
        std::sort(lowVals.begin(), lowVals.end());
        straight = (lowVals[4] - lowVals[0] == 4 && freq.size() == 5);
    }

    bool royalStraight = straight && vals[0] == static_cast<int>(Rank::Ten);

    if (flush && royalStraight) return PokerRank::RoyalFlush;
    if (flush && straight)      return PokerRank::StraightFlush;
    if (counts[0] == 4)         return PokerRank::FourOfAKind;
    if (counts[0] == 3 && counts[1] == 2) return PokerRank::FullHouse;
    if (flush)                  return PokerRank::Flush;
    if (straight)               return PokerRank::Straight;
    if (counts[0] == 3)         return PokerRank::ThreeOfAKind;
    if (counts[0] == 2 && counts[1] == 2) return PokerRank::TwoPair;

    // Pair: check if it's jacks or better (J=11, Q=12, K=13, A=14)
    if (counts[0] == 2) {
        for (auto& kv : freq) {
            if (kv.second == 2 && kv.first >= static_cast<int>(Rank::Jack))
                return PokerRank::JacksOrBetter;
        }
    }
    return PokerRank::None;
}
