#pragma once
#include <string>
#include <vector>
#include <array>

enum class Achievement {
    FirstWin = 0,     // Win any game
    Blackjack,        // Natural blackjack
    SlotJackpot,      // 3x Crown on slots
    LuckyNumber,      // Win a straight-number bet on roulette
    Survivor,         // Survive Russian Roulette (any bullets)
    Daredevil,        // Survive RR with 5 bullets loaded
    HighRoller,       // Place a single bet of $1000 or more
    Millionaire,      // Reach $10000 balance
    Broke,            // Reach $0 balance
    RoyalFlush,       // Get Royal Flush in Video Poker
    SplitWin,         // Win both hands after a Blackjack split
    COUNT
};

struct AchievementInfo {
    std::string title;
    std::string description;
};

class AchievementManager
{
public:
    static AchievementManager& instance();

    bool isUnlocked(Achievement a) const;
    // Returns true if this is a new unlock
    bool unlock(Achievement a);

    const std::vector<Achievement>& getNewlyUnlocked() const { return newlyUnlocked; }
    void clearNewlyUnlocked() { newlyUnlocked.clear(); }

    static AchievementInfo getInfo(Achievement a);

    void save();
    void load();

private:
    AchievementManager();
    std::array<bool, static_cast<int>(Achievement::COUNT)> unlocked{};
    std::vector<Achievement> newlyUnlocked;
};
