#pragma once
#include "IGame.h"
#include "IBlackjackStrategy.h"
#include "../models/Card.h"
#include "../models/Deck.h"
#include <memory>

class Player;

enum class BlackjackPhase { Idle, PlayerTurn, DealerTurn, GameOver };

class Blackjack : public IGame
{
public:
    Blackjack(Player& player, std::unique_ptr<IBlackjackStrategy> strategy);

    void placeBet(double amount) override;
    void play() override;           // deals initial 4 cards
    void hit();
    void stand();
    void doubleDown();

    GameResult getResult() const override { return result; }
    double getPayout() const override { return payout; }
    std::string getName() const override { return "Blackjack"; }

    const Hand& getPlayerHand() const { return playerHand; }
    const Hand& getDealerHand() const { return dealerHand; }
    BlackjackPhase getPhase() const { return phase; }
    double getBet() const { return bet; }
    bool canDoubleDown() const;

    std::string getStrategyName() const;

private:
    Player& player;
    Deck deck;
    Hand playerHand;
    Hand dealerHand;
    double bet   = 0.0;
    double payout = 0.0;
    GameResult result = GameResult::Loss;
    BlackjackPhase phase = BlackjackPhase::Idle;
    std::unique_ptr<IBlackjackStrategy> strategy;

    void dealerPlay();
    void resolve();
};
