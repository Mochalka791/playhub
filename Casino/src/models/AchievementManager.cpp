#include "AchievementManager.h"
#include <fstream>
#include <string>

AchievementManager& AchievementManager::instance()
{
    static AchievementManager inst;
    return inst;
}

AchievementManager::AchievementManager()
{
    load();
}

bool AchievementManager::isUnlocked(Achievement a) const
{
    return unlocked[static_cast<int>(a)];
}

bool AchievementManager::unlock(Achievement a)
{
    int idx = static_cast<int>(a);
    if (unlocked[idx]) return false;
    unlocked[idx] = true;
    newlyUnlocked.push_back(a);
    save();
    return true;
}

AchievementInfo AchievementManager::getInfo(Achievement a)
{
    switch (a) {
    case Achievement::FirstWin:    return {"First Win",       "Win any game for the first time."};
    case Achievement::Blackjack:   return {"Blackjack!",      "Get a natural 21 on the first two cards."};
    case Achievement::SlotJackpot: return {"Slot Jackpot",    "Hit 3x Crown on the slot machine."};
    case Achievement::LuckyNumber: return {"Lucky Number",    "Win a straight-number bet on Roulette."};
    case Achievement::Survivor:    return {"Survivor",        "Survive Russian Roulette."};
    case Achievement::Daredevil:   return {"Daredevil",       "Survive Russian Roulette with 5 bullets loaded."};
    case Achievement::HighRoller:  return {"High Roller",     "Place a single bet of $1000 or more."};
    case Achievement::Millionaire: return {"Millionaire",     "Reach a balance of $10,000."};
    case Achievement::Broke:       return {"Broke",           "Lose everything. Better luck next time."};
    case Achievement::RoyalFlush:  return {"Royal Flush",     "Hit a Royal Flush in Video Poker."};
    case Achievement::SplitWin:    return {"Split Winner",    "Win both hands after a Blackjack split."};
    default: return {"?", "?"};
    }
}

void AchievementManager::save()
{
    std::ofstream f("casino_achievements.dat");
    if (!f) return;
    for (int i = 0; i < static_cast<int>(Achievement::COUNT); ++i)
        f << i << "=" << (unlocked[i] ? 1 : 0) << "\n";
}

void AchievementManager::load()
{
    std::ifstream f("casino_achievements.dat");
    if (!f) return;
    std::string line;
    while (std::getline(f, line)) {
        auto eq = line.find('=');
        if (eq == std::string::npos) continue;
        int idx = std::stoi(line.substr(0, eq));
        int val = std::stoi(line.substr(eq + 1));
        if (idx >= 0 && idx < static_cast<int>(Achievement::COUNT))
            unlocked[idx] = (val == 1);
    }
}
