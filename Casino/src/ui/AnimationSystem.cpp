#include "AnimationSystem.h"
#include <algorithm>
#include <random>

void ReelAnimation::update(float dt)
{
    if (!active) return;
    timer += dt;
    ++frameCounter;
    if (timer >= duration) {
        timer  = duration;
        active = false;
    }
}

void RouletteAnimation::update(float dt)
{
    if (!active) return;
    timer += dt;
    if (timer >= duration) {
        timer  = duration;
        active = false;
        return;
    }
    // Update the rapidly spinning display number
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<int> dist(0, 36);
    displayNumber = dist(rng);
}
