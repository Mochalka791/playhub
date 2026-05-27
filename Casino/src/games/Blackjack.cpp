#include "Blackjack.h"
#include "../models/Player.h"
#include "../core/EventSystem.h"
#include <stdexcept>

Blackjack::Blackjack(Player& player, std::unique_ptr<IBlackjackStrategy> strat)
    : player(player), strategy(std::move(strat))
{}

void Blackjack::placeBet(double amount)
{
    if (!player.canBet(amount))
        throw std::runtime_error("Insufficient balance");
    bet = amount;
}

void Blackjack::play()
{
    if (bet <= 0.0)
        throw std::runtime_error("Place a bet first");

    playerHand.clear();
    dealerHand.clear();
    splitHand.clear();
    splitActive  = false;
    splitPayout  = 0.0;
    splitResult  = GameResult::Loss;
    insuranceBet = 0.0;
    insuranceWin = false;
    phase = BlackjackPhase::PlayerTurn;

    // Deal: player, dealer, player, dealer
    playerHand.push_back(deck.draw());
    dealerHand.push_back(deck.draw());
    playerHand.push_back(deck.draw());
    dealerHand.push_back(deck.draw());

    EventBus::instance().publish(GameStartEvent{player.getName(), getName()});

    // Insurance offer: dealer shows Ace
    if (dealerHand[0].rank == Rank::Ace) {
        phase = BlackjackPhase::InsurancePending;
        return;
    }

    // Check for immediate blackjack
    if (isBlackjack(playerHand)) {
        phase = BlackjackPhase::DealerTurn;
        dealerPlay();
    }
}

bool Blackjack::isInsuranceAvailable() const
{
    return phase == BlackjackPhase::InsurancePending;
}

void Blackjack::takeInsurance()
{
    if (phase != BlackjackPhase::InsurancePending) return;
    insuranceBet = bet * 0.5;
    if (!player.canBet(insuranceBet)) insuranceBet = 0.0;
    else player.addLoss(insuranceBet); // deduct now; restore on win

    // Continue to player turn
    phase = BlackjackPhase::PlayerTurn;
    if (isBlackjack(playerHand)) {
        phase = BlackjackPhase::DealerTurn;
        dealerPlay();
    }
}

void Blackjack::declineInsurance()
{
    if (phase != BlackjackPhase::InsurancePending) return;
    insuranceBet = 0.0;
    phase = BlackjackPhase::PlayerTurn;
    if (isBlackjack(playerHand)) {
        phase = BlackjackPhase::DealerTurn;
        dealerPlay();
    }
}

bool Blackjack::canSplit() const
{
    return phase == BlackjackPhase::PlayerTurn
        && playerHand.size() == 2
        && playerHand[0].rank == playerHand[1].rank
        && player.canBet(bet)
        && !splitActive;
}

void Blackjack::split()
{
    if (!canSplit()) return;
    player.addLoss(bet);        // extra bet for split hand (refunded on win)
    splitHand.clear();
    splitHand.push_back(playerHand[1]);
    playerHand.resize(1);
    playerHand.push_back(deck.draw());
    splitHand.push_back(deck.draw());
    splitActive = true;
    // Continue playing playerHand first, SplitTurn comes after stand()
}

void Blackjack::hit()
{
    if (phase == BlackjackPhase::PlayerTurn) {
        playerHand.push_back(deck.draw());
        if (isBust(playerHand)) {
            if (splitActive) {
                phase = BlackjackPhase::SplitTurn; // move to split hand
            } else {
                phase = BlackjackPhase::DealerTurn;
                resolve();
            }
        }
    } else if (phase == BlackjackPhase::SplitTurn) {
        splitHand.push_back(deck.draw());
        if (isBust(splitHand)) {
            phase = BlackjackPhase::DealerTurn;
            dealerPlay();
        }
    }
}

void Blackjack::stand()
{
    if (phase == BlackjackPhase::PlayerTurn) {
        if (splitActive) {
            phase = BlackjackPhase::SplitTurn;
        } else {
            phase = BlackjackPhase::DealerTurn;
            dealerPlay();
        }
    } else if (phase == BlackjackPhase::SplitTurn) {
        phase = BlackjackPhase::DealerTurn;
        dealerPlay();
    }
}

void Blackjack::doubleDown()
{
    if (!canDoubleDown()) return;
    if (!player.canBet(bet))
        throw std::runtime_error("Insufficient balance for double down");
    bet *= 2.0;
    playerHand.push_back(deck.draw());
    phase = BlackjackPhase::DealerTurn;
    if (!isBust(playerHand))
        dealerPlay();
    else
        resolve();
}

bool Blackjack::canDoubleDown() const
{
    return (phase == BlackjackPhase::PlayerTurn || phase == BlackjackPhase::SplitTurn)
        && (phase == BlackjackPhase::PlayerTurn ? playerHand.size() : splitHand.size()) == 2
        && player.canBet(bet);
}

std::string Blackjack::getStrategyName() const
{
    return strategy->getName();
}

void Blackjack::dealerPlay()
{
    phase = BlackjackPhase::DealerTurn;
    // Dealer draws until strategy says stop or bust
    while (strategy->shouldDealerHit(handValue(dealerHand), isSoftHand(dealerHand))) {
        dealerHand.push_back(deck.draw());
        if (isBust(dealerHand)) break;
    }
    resolve();
}

void Blackjack::resolve()
{
    phase = BlackjackPhase::GameOver;

    int pv = handValue(playerHand);
    int dv = handValue(dealerHand);
    bool pBust = isBust(playerHand);
    bool dBust = isBust(dealerHand);
    bool pBJ   = isBlackjack(playerHand);
    bool dBJ   = isBlackjack(dealerHand);

    // Insurance payout
    if (insuranceBet > 0.0) {
        if (dBJ) {
            insuranceWin = true;
            player.addWin(insuranceBet * 3.0); // 2:1 + stake back
        }
        // If no dealer BJ, insurance bet was already deducted — stays lost
    }

    if (pBust) {
        result = GameResult::Loss;
        payout = 0.0;
        player.addLoss(bet);
        EventBus::instance().publish(LossEvent{player.getName(), bet, getName()});
    } else if (dBust) {
        result = GameResult::Win;
        payout = bet * 2.0;
        player.addWin(bet);
        EventBus::instance().publish(WinEvent{player.getName(), bet, getName()});
    } else if (pBJ && !dBJ) {
        result = GameResult::Blackjack;
        payout = bet * 2.5;
        player.addWin(bet * 1.5);
        EventBus::instance().publish(WinEvent{player.getName(), bet * 1.5, getName()});
    } else if (dBJ && !pBJ) {
        result = GameResult::Loss;
        payout = 0.0;
        player.addLoss(bet);
        EventBus::instance().publish(LossEvent{player.getName(), bet, getName()});
    } else if (pv > dv) {
        result = GameResult::Win;
        payout = bet * 2.0;
        player.addWin(bet);
        EventBus::instance().publish(WinEvent{player.getName(), bet, getName()});
    } else if (dv > pv) {
        result = GameResult::Loss;
        payout = 0.0;
        player.addLoss(bet);
        EventBus::instance().publish(LossEvent{player.getName(), bet, getName()});
    } else {
        result = GameResult::Push;
        payout = bet;
        EventBus::instance().publish(GameEndEvent{player.getName(), getName(), 0.0});
        if (splitActive) resolveSplit();
        return;
    }

    if (splitActive) resolveSplit();

    double netChange = (result == GameResult::Loss) ? -bet : payout - bet;
    EventBus::instance().publish(GameEndEvent{player.getName(), getName(), netChange});
}

void Blackjack::resolveSplit()
{
    int sv = handValue(splitHand);
    int dv = handValue(dealerHand);
    bool sBust = isBust(splitHand);
    bool dBust = isBust(dealerHand);

    if (sBust || (!dBust && dv > sv)) {
        splitResult = GameResult::Loss;
        splitPayout = 0.0;
        player.addLoss(bet);
    } else if (dBust || sv > dv) {
        splitResult = GameResult::Win;
        splitPayout = bet * 2.0;
        player.addWin(bet);
    } else {
        splitResult = GameResult::Push;
        splitPayout = bet;
    }
}
