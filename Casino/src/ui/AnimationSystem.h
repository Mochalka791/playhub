#pragma once

// Simple per-frame timer helpers used by individual states.

struct ReelAnimation
{
    bool  active      = false;
    float timer       = 0.0f;
    float duration    = 2.5f;   // total spin time in seconds

    // Each reel stops at this fraction of duration
    float stop1 = 0.50f;
    float stop2 = 0.72f;
    float stop3 = 1.00f;

    int   frameCounter = 0;     // incremented each update tick

    void start()  { active = true; timer = 0.0f; frameCounter = 0; }
    void stop()   { active = false; }
    void update(float dt);

    float progress()  const { return timer / duration; }
    bool  reel1Done() const { return progress() >= stop1; }
    bool  reel2Done() const { return progress() >= stop2; }
    bool  reel3Done() const { return progress() >= stop3; }
    bool  finished()  const { return timer >= duration; }
};

struct RouletteAnimation
{
    bool  active       = false;
    float timer        = 0.0f;
    float duration     = 3.0f;
    int   displayNumber = 0;     // rapidly changing number

    void start()  { active = true; timer = 0.0f; displayNumber = 0; }
    void stop()   { active = false; }
    void update(float dt);

    float progress() const { return timer / duration; }
    bool  finished() const { return timer >= duration; }
};
