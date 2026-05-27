#pragma once
#include "IGame.h"
#include "../models/Card.h"
#include "../models/Deck.h"
#include <array>
#include <string>

class Player;

enum class PokerRank {
    None = 0,
    JacksOrBetter,
    TwoPair,
    ThreeOfAKind,
    Straight,
    Flush,
    FullHouse,
    FourOfAKind,
    StraightFlush,
    RoyalFlush
};

class VideoPoker : public IGame
{
public:
    enum class Phase { Idle, Holding, GameOver };

    explicit VideoPoker(Player& player);

    void placeBet(double amount) override;
    void play() override;   // deal 5 cards
    void setHold(int idx, bool hold);
    void draw();            // replace non-held, evaluate, resolve

    GameResult getResult() const override { return result; }
    double getPayout() const override { return payout; }
    std::string getName() const override { return "Video Poker"; }

    Phase      getPhase()    const { return phase; }
    PokerRank  getHandRank() const { return handRank; }
    double     getBet()      const { return bet; }

    const std::array<Card, 5>& getHand()  const { return hand; }
    const std::array<bool, 5>& getHolds() const { return holds; }

    static std::string rankName(PokerRank r);
    static double      multiplier(PokerRank r);

private:
    Player& player;
    Deck    deck;

    std::array<Card, 5> hand{};
    std::array<bool, 5> holds{};
    double     bet      = 0.0;
    double     payout   = 0.0;
    Phase      phase    = Phase::Idle;
    PokerRank  handRank = PokerRank::None;
    GameResult result   = GameResult::Loss;

    static PokerRank evaluate(const std::array<Card, 5>& h);
};
