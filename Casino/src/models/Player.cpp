#include "Player.h"
#include <stdexcept>

Player::Player(const std::string& name, double startBalance)
    : name(name), balance(startBalance)
{
    if (startBalance < 0.0)
        throw std::invalid_argument("Start balance cannot be negative");
}

void Player::addWin(double amount)
{
    balance     += amount;
    totalWon    += amount;
    ++gamesPlayed;
}

void Player::addLoss(double amount)
{
    balance  -= amount;
    totalLost += amount;
    ++gamesPlayed;
}
