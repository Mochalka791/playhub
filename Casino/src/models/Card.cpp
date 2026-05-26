#include "Card.h"
#include <algorithm>
#include <stdexcept>

int Card::baseValue() const
{
    int r = static_cast<int>(rank);
    if (r >= 2 && r <= 10) return r;
    if (rank == Rank::Ace) return 11;
    return 10; // Jack, Queen, King
}

std::string Card::rankStr() const
{
    switch (rank) {
        case Rank::Two:   return "2";
        case Rank::Three: return "3";
        case Rank::Four:  return "4";
        case Rank::Five:  return "5";
        case Rank::Six:   return "6";
        case Rank::Seven: return "7";
        case Rank::Eight: return "8";
        case Rank::Nine:  return "9";
        case Rank::Ten:   return "10";
        case Rank::Jack:  return "J";
        case Rank::Queen: return "Q";
        case Rank::King:  return "K";
        case Rank::Ace:   return "A";
    }
    return "?";
}

std::string Card::suitSymbol() const
{
    switch (suit) {
        case Suit::Hearts:   return "H";
        case Suit::Diamonds: return "D";
        case Suit::Clubs:    return "C";
        case Suit::Spades:   return "S";
    }
    return "?";
}

std::string Card::toString() const
{
    return rankStr() + suitSymbol();
}

// ---- Free functions ----------------------------------------

int handValue(const Hand& hand)
{
    int value = 0;
    int aces  = 0;
    for (const auto& c : hand) {
        value += c.baseValue();
        if (c.rank == Rank::Ace) ++aces;
    }
    // Reduce Aces from 11 to 1 to avoid bust
    while (value > 21 && aces > 0) {
        value -= 10;
        --aces;
    }
    return value;
}

bool isBlackjack(const Hand& hand)
{
    return hand.size() == 2 && handValue(hand) == 21;
}

bool isBust(const Hand& hand)
{
    return handValue(hand) > 21;
}

bool isSoftHand(const Hand& hand)
{
    // A hand is "soft" if it contains an Ace counted as 11
    int value = 0;
    int aces  = 0;
    for (const auto& c : hand) {
        value += c.baseValue();
        if (c.rank == Rank::Ace) ++aces;
    }
    // If value <= 21 and we still have an Ace counted as 11, it's soft
    return aces > 0 && value <= 21;
}
