#include "GameFactory.h"
#include "../games/SlotMachine.h"
#include "../games/Blackjack.h"
#include "../games/Roulette.h"
#include "../games/RussianRoulette.h"
#include "../games/DealerHitSoft17.h"
#include "../games/DealerStandSoft17.h"
#include <stdexcept>

std::unique_ptr<IGame> GameFactory::createGame(GameType type,
                                                Player& player,
                                                const std::string& option)
{
    switch (type) {
    case GameType::SlotMachine:
        return std::make_unique<SlotMachine>(player);

    case GameType::Blackjack: {
        std::unique_ptr<IBlackjackStrategy> strategy;
        if (option == "StandSoft17")
            strategy = std::make_unique<DealerStandSoft17>();
        else
            strategy = std::make_unique<DealerHitSoft17>();
        return std::make_unique<Blackjack>(player, std::move(strategy));
    }

    case GameType::Roulette:
        return std::make_unique<Roulette>(player);

    case GameType::RussianRoulette:
        return std::make_unique<RussianRoulette>(player);

    default:
        throw std::invalid_argument("Unknown game type");
    }
}
