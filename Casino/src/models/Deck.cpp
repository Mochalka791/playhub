#include "Deck.h"
#include <algorithm>
#include <random>
#include <stdexcept>

Deck::Deck()
{
    build();
    shuffle();
}

void Deck::build()
{
    cards.clear();
    top = 0;
    for (int s = 0; s < 4; ++s) {
        for (int r = 2; r <= 14; ++r) {
            Card c;
            c.suit = static_cast<Suit>(s);
            c.rank = static_cast<Rank>(r);
            cards.push_back(c);
        }
    }
}

void Deck::reset()
{
    build();
    shuffle();
}

void Deck::shuffle()
{
    std::random_device rd;
    std::mt19937 rng(rd());
    std::shuffle(cards.begin() + top, cards.end(), rng);
}

Card Deck::draw()
{
    if (isEmpty()) {
        reset();
    }
    return cards[top++];
}

bool Deck::isEmpty() const
{
    return top >= static_cast<int>(cards.size());
}

int Deck::remaining() const
{
    return static_cast<int>(cards.size()) - top;
}
