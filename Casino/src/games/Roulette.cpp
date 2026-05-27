#include "Roulette.h"
#include "../models/Player.h"
#include "../core/EventSystem.h"
#include <random>
#include <numeric>
#include <stdexcept>

// European roulette: 0-36, red numbers
static const int RED_NUMBERS[] = {
    1,3,5,7,9,12,14,16,18,19,21,23,25,27,30,32,34,36
};

Roulette::Roulette(Player& player) : player(player) {}

bool Roulette::isRedNumber(int n) const
{
    for (int r : RED_NUMBERS)
        if (r == n) return true;
    return false;
}

void Roulette::addBet(BetType type, double amount, int number)
{
    bets.push_back({ type, amount, number });
}

void Roulette::clearBets()
{
    bets.clear();
}

double Roulette::getTotalBetAmount() const
{
    double total = 0.0;
    for (const auto& b : bets) total += b.amount;
    return total;
}

void Roulette::placeBet(double amount)
{
    addBet(currentBetType, amount, straightTarget);
}

void Roulette::play()
{
    double totalBet = getTotalBetAmount();
    if (totalBet <= 0.0)
        throw std::runtime_error("No bets placed");
    if (!player.canBet(totalBet))
        throw std::runtime_error("Insufficient balance");

    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<int> dist(0, 36);
    lastNumber = dist(rng);

    totalPayout = 0.0;
    for (const auto& b : bets)
        totalPayout += calcBetPayout(b, lastNumber);

    double net = totalPayout - totalBet;
    if (net > 0.0) {
        result = GameResult::Win;
        player.addWin(net);
        EventBus::instance().publish(WinEvent{player.getName(), net, getName()});
    } else if (net < 0.0) {
        result = GameResult::Loss;
        player.addLoss(-net);
        EventBus::instance().publish(LossEvent{player.getName(), -net, getName()});
    } else {
        result = GameResult::Push;
    }

    EventBus::instance().publish(GameEndEvent{player.getName(), getName(), net});
    bets.clear();
}

double Roulette::calcBetPayout(const RouletteBet& b, int number) const
{
    switch (b.type) {
    case BetType::Red:
        return isRedNumber(number) ? b.amount * 2.0 : 0.0;
    case BetType::Black:
        return (!isRedNumber(number) && number != 0) ? b.amount * 2.0 : 0.0;
    case BetType::Even:
        return (number != 0 && number % 2 == 0) ? b.amount * 2.0 : 0.0;
    case BetType::Odd:
        return (number % 2 == 1) ? b.amount * 2.0 : 0.0;
    case BetType::Dozen1:
        return (number >= 1  && number <= 12) ? b.amount * 3.0 : 0.0;
    case BetType::Dozen2:
        return (number >= 13 && number <= 24) ? b.amount * 3.0 : 0.0;
    case BetType::Dozen3:
        return (number >= 25 && number <= 36) ? b.amount * 3.0 : 0.0;
    case BetType::Straight:
        return (number == b.straightNumber) ? b.amount * 36.0 : 0.0;
    case BetType::Low:
        return (number >= 1 && number <= 18) ? b.amount * 2.0 : 0.0;
    case BetType::High:
        return (number >= 19 && number <= 36) ? b.amount * 2.0 : 0.0;
    }
    return 0.0;
}
