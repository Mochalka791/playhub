#include "HighscoreManager.h"
#include "Player.h"
#include <algorithm>

HighscoreManager& HighscoreManager::instance()
{
    static HighscoreManager inst;
    return inst;
}

void HighscoreManager::submit(const Player& player)
{
    // Check for existing entry with same name and update
    for (auto& e : entries) {
        if (e.name == player.getName()) {
            e.balance     = player.getBalance();
            e.netProfit   = player.getNetProfit();
            e.gamesPlayed = player.getGamesPlayed();
            std::sort(entries.begin(), entries.end(),
                [](const HighscoreEntry& a, const HighscoreEntry& b) {
                    return a.netProfit > b.netProfit;
                });
            return;
        }
    }

    HighscoreEntry entry;
    entry.name        = player.getName();
    entry.balance     = player.getBalance();
    entry.netProfit   = player.getNetProfit();
    entry.gamesPlayed = player.getGamesPlayed();
    entries.push_back(entry);

    std::sort(entries.begin(), entries.end(),
        [](const HighscoreEntry& a, const HighscoreEntry& b) {
            return a.netProfit > b.netProfit;
        });

    if (static_cast<int>(entries.size()) > MAX_ENTRIES)
        entries.resize(MAX_ENTRIES);
}

void HighscoreManager::clear()
{
    entries.clear();
}
