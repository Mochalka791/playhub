#pragma once
#include <string>

enum class GameResult { Win, Loss, Push, Blackjack };

class IGame
{
public:
    virtual ~IGame() = default;
    virtual void placeBet(double amount) = 0;
    virtual void play() = 0;
    virtual GameResult getResult() const = 0;
    virtual double getPayout() const = 0;
    virtual std::string getName() const = 0;
};
