#include "AppSettings.h"
#include "../audio/AudioManager.h"
#include <fstream>
#include <sstream>
#include <chrono>
#include <ctime>

AppSettings& AppSettings::instance()
{
    static AppSettings inst;
    return inst;
}

AppSettings::AppSettings()
{
    load();
}

static std::string todayString()
{
    auto now  = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    struct tm tm_info;
#ifdef _WIN32
    localtime_s(&tm_info, &t);
#else
    localtime_r(&t, &tm_info);
#endif
    char buf[16];
    snprintf(buf, sizeof(buf), "%04d-%02d-%02d",
             tm_info.tm_year + 1900, tm_info.tm_mon + 1, tm_info.tm_mday);
    return std::string(buf);
}

std::string AppSettings::getSettingsPath()
{
    return "casino_settings.dat";
}

void AppSettings::save()
{
    std::ofstream f(getSettingsPath());
    if (!f) return;
    f << "volume="         << masterVolume   << "\n";
    f << "fullscreen="     << (fullscreen ? 1 : 0) << "\n";
    f << "dealerStrategy=" << dealerStrategy << "\n";
    f << "minBet="         << minBet         << "\n";
    f << "maxBet="         << maxBet         << "\n";
    f << "lastBonusDate="  << lastBonusDate  << "\n";
}

void AppSettings::load()
{
    std::ifstream f(getSettingsPath());
    if (!f) return;
    std::string line;
    while (std::getline(f, line)) {
        auto eq = line.find('=');
        if (eq == std::string::npos) continue;
        std::string key = line.substr(0, eq);
        std::string val = line.substr(eq + 1);
        if (key == "volume")         masterVolume   = std::stoi(val);
        else if (key == "fullscreen")fullscreen     = (val == "1");
        else if (key == "dealerStrategy") dealerStrategy = std::stoi(val);
        else if (key == "minBet")    minBet         = std::stof(val);
        else if (key == "maxBet")    maxBet         = std::stof(val);
        else if (key == "lastBonusDate") lastBonusDate = val;
    }
    // Apply saved volume immediately
    AudioManager::instance().setMasterVolume(masterVolume);
}

bool AppSettings::tryClaimDailyBonus()
{
    std::string today = todayString();
    if (lastBonusDate == today) return false;
    lastBonusDate    = today;
    dailyBonusPending = true;
    save();
    return true;
}
