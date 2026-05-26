#pragma once
#include "IGame.h"
#include <vector>
#include <string>

class Player;

enum class BetType { Red, Black, Even, Odd, Dozen1, Dozen2, Dozen3, Straight };

struct RouletteBet
{
    BetType type;
    double  amount;
    int     straightNumber = -1;    // only used for Straight bets
};

class Roulette : public IGame
{
public:
    explicit Roulette(Player& player);

    // IGame
    void placeBet(double amount) override;  // applies amount to currentBetType
    void play() override;
    GameResult getResult() const override { return result; }
    double getPayout() const override { return totalPayout; }
    std::string getName() const override { return "Roulette"; }

    // Roulette-specific
    void addBet(BetType type, double amount, int number = -1);
    void clearBets();
    double getTotalBetAmount() const;
    const std::vector<RouletteBet>& getBets() const { return bets; }

    int  getLastNumber() const { return lastNumber; }
    bool isRedNumber(int n) const;

    BetType currentBetType = BetType::Red;
    int     straightTarget = 0;

private:
    Player& player;
    std::vector<RouletteBet> bets;
    int    lastNumber  = -1;
    double totalPayout = 0.0;
    GameResult result  = GameResult::Loss;

    double calcBetPayout(const RouletteBet& b, int number) const;
};
