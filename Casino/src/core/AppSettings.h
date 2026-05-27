#pragma once
#include <string>

class AppSettings
{
public:
    static AppSettings& instance();

    // Audio
    int  masterVolume   = 100;   // 0-128

    // Display
    bool fullscreen     = false;

    // Blackjack
    int  dealerStrategy = 0;     // 0=HitSoft17, 1=StandSoft17

    // Table limits (shared across games)
    float minBet        = 1.0f;
    float maxBet        = 10000.0f;

    void save();
    void load();

    // Returns true exactly once per calendar day; awards $100 bonus
    bool tryClaimDailyBonus();
    bool dailyBonusPending = false; // set after load, cleared once shown

private:
    AppSettings();
    std::string lastBonusDate;

    static std::string getSettingsPath();
};
