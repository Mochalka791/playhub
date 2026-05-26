#pragma once
#include <memory>
#include <string>

class IGame;
class Player;

enum class GameType { SlotMachine, Blackjack, Roulette, RussianRoulette };

class GameFactory
{
public:
    // dealerStrategy only relevant for Blackjack ("HitSoft17" | "StandSoft17")
    static std::unique_ptr<IGame> createGame(GameType type,
                                             Player& player,
                                             const std::string& option = "HitSoft17");
};
