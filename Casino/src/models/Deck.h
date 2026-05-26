#pragma once
#include "Card.h"
#include <vector>

class Deck
{
public:
    Deck();
    void reset();
    void shuffle();
    Card draw();
    bool isEmpty() const;
    int remaining() const;

private:
    std::vector<Card> cards;
    int top = 0;

    void build();
};
