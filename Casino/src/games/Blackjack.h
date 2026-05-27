#pragma once
#include "IGame.h"
#include "IBlackjackStrategy.h"
#include "../models/Card.h"
#include "../models/Deck.h"
#include <memory>

class Player;

enum class BlackjackPhase { Idle, InsurancePending, PlayerTurn, SplitTurn, DealerTurn, GameOver };

class Blackjack : public IGame
{
public:
    Blackjack(Player& player, std::unique_ptr<IBlackjackStrategy> strategy);

    void placeBet(double amount) override;
    void play() override;           // deals initial 4 cards
    void hit();
    void stand();
    void doubleDown();

    // Split
    bool canSplit() const;
    void split();

    // Insurance
    bool isInsuranceAvailable() const;
    void takeInsurance();
    void declineInsurance();

    GameResult getResult() const override { return result; }
    double getPayout() const override { return payout; }
    std::string getName() const override { return "Blackjack"; }

    const Hand& getPlayerHand() const { return playerHand; }
    const Hand& getDealerHand() const { return dealerHand; }
    BlackjackPhase getPhase() const { return phase; }
    double getBet() const { return bet; }
    bool canDoubleDown() const;

    // Split hand accessors
    bool        hasSplit()      const { return splitActive; }
    const Hand& getSplitHand()  const { return splitHand; }
    GameResult  getSplitResult()const { return splitResult; }
    double      getSplitPayout()const { return splitPayout; }

    // Insurance accessors
    double getInsuranceBet() const { return insuranceBet; }
    bool   insuranceWon()    const { return insuranceWin; }

    std::string getStrategyName() const;

private:
    Player& player;
    Deck deck;
    Hand playerHand;
    Hand dealerHand;
    double bet    = 0.0;
    double payout = 0.0;
    GameResult     result = GameResult::Loss;
    BlackjackPhase phase  = BlackjackPhase::Idle;
    std::unique_ptr<IBlackjackStrategy> strategy;

    // Split
    bool       splitActive  = false;
    Hand       splitHand;
    double     splitPayout  = 0.0;
    GameResult splitResult  = GameResult::Loss;

    // Insurance
    double insuranceBet = 0.0;
    bool   insuranceWin = false;

    void dealerPlay();
    void resolve();
    void resolveSplit();
};
