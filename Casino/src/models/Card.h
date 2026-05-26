#pragma once
#include <string>
#include <vector>

enum class Suit { Hearts, Diamonds, Clubs, Spades };
enum class Rank { Two=2, Three, Four, Five, Six, Seven, Eight, Nine, Ten, Jack, Queen, King, Ace };

struct Card
{
    Suit suit;
    Rank rank;

    // Base value (Ace = 11, face cards = 10)
    int baseValue() const;
    std::string rankStr() const;
    std::string suitSymbol() const;
    std::string toString() const;
};

using Hand = std::vector<Card>;

// Calculates best hand value (handles Ace as 1 or 11)
int handValue(const Hand& hand);
bool isBlackjack(const Hand& hand);
bool isBust(const Hand& hand);
bool isSoftHand(const Hand& hand);
