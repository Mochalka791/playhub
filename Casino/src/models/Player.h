#pragma once
#include <string>

class Player
{
public:
    Player(const std::string& name, double startBalance);

    const std::string& getName() const { return name; }
    double getBalance() const { return balance; }
    int getGamesPlayed() const { return gamesPlayed; }
    double getTotalWon() const { return totalWon; }
    double getTotalLost() const { return totalLost; }
    double getNetProfit() const { return totalWon - totalLost; }

    void addWin(double amount);
    void addLoss(double amount);
    bool canBet(double amount) const { return balance >= amount; }

private:
    std::string name;
    double balance;
    int gamesPlayed = 0;
    double totalWon = 0.0;
    double totalLost = 0.0;
};
