#include "RussianRoulette.h"
#include "../models/Player.h"
#include "../core/EventSystem.h"
#include <random>
#include <algorithm>
#include <stdexcept>

RussianRoulette::RussianRoulette(Player& player) : player(player)
{
    chambers.fill(false);
}

void RussianRoulette::setBullets(int n)
{
    bullets = std::clamp(n, 1, 5);
}

void RussianRoulette::placeBet(double amount)
{
    if (!player.canBet(amount))
        throw std::runtime_error("Insufficient balance");
    bet = amount;
}

double RussianRoulette::getMultiplier() const
{
    // Higher risk = higher multiplier
    switch (bullets) {
        case 1: return 1.5;
        case 2: return 2.5;
        case 3: return 5.0;
        case 4: return 10.0;
        case 5: return 20.0;
        default: return 1.5;
    }
}

void RussianRoulette::play()
{
    if (bet <= 0.0)
        throw std::runtime_error("Place a bet first");

    std::mt19937 rng(std::random_device{}());

    // Load bullets into first N chambers, then shuffle
    chambers.fill(false);
    for (int i = 0; i < bullets; ++i)
        chambers[i] = true;
    std::shuffle(chambers.begin(), chambers.end(), rng);

    // Pull trigger: pick a random chamber
    std::uniform_int_distribution<int> dist(0, 5);
    firedChamber = dist(rng);
    shot = chambers[firedChamber];

    if (!shot) {
        result = GameResult::Win;
        payout = bet * getMultiplier();
        player.addWin(payout - bet);
        EventBus::instance().publish(WinEvent{player.getName(), payout - bet, getName()});
    } else {
        result = GameResult::Loss;
        payout = 0.0;
        player.addLoss(bet);
        EventBus::instance().publish(LossEvent{player.getName(), bet, getName()});
    }

    EventBus::instance().publish(GameEndEvent{
        player.getName(), getName(), shot ? -bet : payout - bet
    });
}
