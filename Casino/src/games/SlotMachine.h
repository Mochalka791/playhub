#pragma once
#include "IGame.h"
#include <array>
#include <string>

class Player;

enum class SlotSymbol { Crown, Seven, Diamond, Bar, Bell, Star, Cherry, Lemon, Orange, Clover, COUNT };

class SlotMachine : public IGame
{
public:
    explicit SlotMachine(Player& player);

    void placeBet(double amount) override;
    void play() override;
    GameResult getResult() const override { return result; }
    double getPayout() const override { return payout; }
    std::string getName() const override { return "Slot Machine"; }

    const std::array<SlotSymbol, 3>& getReels() const { return reels; }
    double getBet() const { return bet; }

    // Freespins: awarded when all 3 reels show Clover (scatter)
    int  getFreeSpinsRemaining() const { return freeSpinsRemaining; }
    bool isFreeSpin()            const { return currentlyFreeSpin; }
    void consumeFreeSpin();  // call before play() on a free spin

    static std::string symbolToString(SlotSymbol s);
    static double      symbolMultiplier(SlotSymbol s);

private:
    Player& player;
    double bet    = 0.0;
    double payout = 0.0;
    GameResult result = GameResult::Loss;
    std::array<SlotSymbol, 3> reels = {};

    int  freeSpinsRemaining = 0;
    bool currentlyFreeSpin  = false;

    void calculateResult();
};
