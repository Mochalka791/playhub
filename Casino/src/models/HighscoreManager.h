#pragma once
#include <string>
#include <vector>

class Player;

struct HighscoreEntry
{
    std::string name;
    double balance;
    double netProfit;
    int gamesPlayed;
};

class HighscoreManager
{
public:
    static HighscoreManager& instance();

    void submit(const Player& player);
    const std::vector<HighscoreEntry>& getEntries() const { return entries; }
    void clear();

    static constexpr int MAX_ENTRIES = 10;

private:
    HighscoreManager() = default;
    std::vector<HighscoreEntry> entries;
};
