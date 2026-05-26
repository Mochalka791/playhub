#pragma once
#include "IGame.h"
#include <array>
#include <string>

class Player;

class RussianRoulette : public IGame
{
public:
    explicit RussianRoulette(Player& player);

    void setBullets(int n);     // 1-5
    void placeBet(double amount) override;
    void play() override;

    GameResult getResult() const override { return result; }
    double getPayout() const override { return payout; }
    std::string getName() const override { return "Russian Roulette"; }

    bool wasShot()          const { return shot; }
    int  getBullets()       const { return bullets; }
    int  getFiredChamber()  const { return firedChamber; }
    double getBet()         const { return bet; }
    double getMultiplier()  const;

    // Which chambers have bullets (randomised after play())
    const std::array<bool, 6>& getChambers() const { return chambers; }

private:
    Player& player;
    int    bullets      = 1;
    double bet          = 0.0;
    double payout       = 0.0;
    bool   shot         = false;
    int    firedChamber = -1;
    GameResult result   = GameResult::Loss;

    std::array<bool, 6> chambers{};
};
