#include "HighscoreManager.h"
#include "Player.h"
#include <algorithm>
#include <fstream>
#include <sstream>

HighscoreManager& HighscoreManager::instance()
{
    static HighscoreManager inst;
    inst.load();  // load once on first access
    return inst;
}

void HighscoreManager::save()
{
    std::ofstream f("casino_scores.dat");
    if (!f) return;
    for (const auto& e : entries)
        f << e.name << "," << e.balance << "," << e.netProfit << "," << e.gamesPlayed << "\n";
}

void HighscoreManager::load()
{
    if (!entries.empty()) return; // already loaded
    std::ifstream f("casino_scores.dat");
    if (!f) return;
    std::string line;
    while (std::getline(f, line)) {
        std::istringstream ss(line);
        std::string tok;
        HighscoreEntry e;
        if (!std::getline(ss, e.name, ',')) continue;
        if (!std::getline(ss, tok, ',')) continue; e.balance     = std::stod(tok);
        if (!std::getline(ss, tok, ',')) continue; e.netProfit   = std::stod(tok);
        if (!std::getline(ss, tok))     continue; e.gamesPlayed = std::stoi(tok);
        entries.push_back(e);
    }
    std::sort(entries.begin(), entries.end(),
        [](const HighscoreEntry& a, const HighscoreEntry& b) {
            return a.netProfit > b.netProfit;
        });
    if ((int)entries.size() > MAX_ENTRIES)
        entries.resize(MAX_ENTRIES);
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
    save();
}

void HighscoreManager::clear()
{
    entries.clear();
}
