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
    phase = BlackjackPhase::PlayerTurn;

    // Deal: player, dealer, player, dealer
    playerHand.push_back(deck.draw());
    dealerHand.push_back(deck.draw());
    playerHand.push_back(deck.draw());
    dealerHand.push_back(deck.draw());

    EventBus::instance().publish(GameStartEvent{player.getName(), getName()});

    // Check for immediate blackjack
    if (isBlackjack(playerHand)) {
        phase = BlackjackPhase::DealerTurn;
        dealerPlay();
    }
}

void Blackjack::hit()
{
    if (phase != BlackjackPhase::PlayerTurn) return;
    playerHand.push_back(deck.draw());
    if (isBust(playerHand)) {
        phase = BlackjackPhase::DealerTurn;
        resolve();
    }
}

void Blackjack::stand()
{
    if (phase != BlackjackPhase::PlayerTurn) return;
    phase = BlackjackPhase::DealerTurn;
    dealerPlay();
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
    return phase == BlackjackPhase::PlayerTurn
        && playerHand.size() == 2
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
        // Balance unchanged on push
        EventBus::instance().publish(
            GameEndEvent{player.getName(), getName(), 0.0});
        return;
    }

    double netChange = (result == GameResult::Loss) ? -bet : payout - bet;
    EventBus::instance().publish(GameEndEvent{player.getName(), getName(), netChange});
}
