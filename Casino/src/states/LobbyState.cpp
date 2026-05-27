#include "LobbyState.h"
#include "MenuState.h"
#include "SlotMachineState.h"
#include "BlackjackState.h"
#include "RouletteState.h"
#include "RussianRouletteState.h"
#include "HighscoreState.h"
#include "SettingsState.h"
#include "VideoPokerState.h"
#include "../core/StateManager.h"
#include "../core/Application.h"
#include "../core/AppSettings.h"
#include "../models/Player.h"
#include "../models/HighscoreManager.h"
#include "../models/AchievementManager.h"
#include "../ui/Theme.h"
#include "../audio/AudioManager.h"
#include <imgui.h>
#include <string>

LobbyState::LobbyState(StateManager& sm, Application& app)
    : IState(sm, app) {}

void LobbyState::onEnter()
{
    if (Player* p = app.getPlayer())
        HighscoreManager::instance().submit(*p);

    // Daily bonus
    if (Player* p = app.getPlayer()) {
        if (AppSettings::instance().tryClaimDailyBonus()) {
            p->addWin(100.0);
            dailyBonusMsg = "Daily bonus: +$100 added to your balance!";
            dailyBonusTimer = 4.0f;
        }
    }
}

void LobbyState::update(float dt)
{
    if (dailyBonusTimer > 0.0f) dailyBonusTimer -= dt;

    // Show newly unlocked achievements briefly
    auto& am = AchievementManager::instance();
    if (!am.getNewlyUnlocked().empty() && achieveTimer <= 0.0f) {
        auto& list = am.getNewlyUnlocked();
        auto info = AchievementManager::getInfo(list[0]);
        achieveMsg   = "Achievement: " + info.title + " - " + info.description;
        achieveTimer = 3.5f;
        am.clearNewlyUnlocked();
    }
    if (achieveTimer > 0.0f) achieveTimer -= dt;
}

void LobbyState::render()
{
    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos({0, 0});
    ImGui::SetNextWindowSize(io.DisplaySize);
    ImGui::Begin("##lobby", nullptr,
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoScrollbar  | ImGuiWindowFlags_NoSavedSettings);

    float cx = io.DisplaySize.x * 0.5f;
    float cy = io.DisplaySize.y * 0.5f;

    // Header
    Theme::PushTitleStyle();
    ImGui::SetWindowFontScale(1.8f);
    ImGui::SetCursorPos({cx - 60.0f, cy - 240.0f});
    ImGui::Text("LOBBY");
    ImGui::SetWindowFontScale(1.0f);
    Theme::PopTitleStyle();

    // Player info
    if (Player* p = app.getPlayer()) {
        ImGui::SetCursorPos({cx - 200.0f, cy - 190.0f});
        ImGui::Text("Welcome, %s!", p->getName().c_str());
        ImGui::SetCursorPos({cx - 200.0f, cy - 162.0f});
        ImGui::PushStyleColor(ImGuiCol_Text, Theme::Gold());
        ImGui::Text("Balance: $%.2f    Net: %+.2f$    Games: %d",
                    p->getBalance(), p->getNetProfit(), p->getGamesPlayed());
        ImGui::PopStyleColor();
    }

    // Daily bonus notification
    if (dailyBonusTimer > 0.0f) {
        ImGui::SetCursorPos({cx - 200.0f, cy - 132.0f});
        ImGui::PushStyleColor(ImGuiCol_Text, Theme::Gold());
        ImGui::Text("%s", dailyBonusMsg.c_str());
        ImGui::PopStyleColor();
    }

    // Achievement notification
    if (achieveTimer > 0.0f) {
        ImGui::SetCursorPos({cx - 260.0f, cy - 110.0f});
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{0.8f, 1.0f, 0.4f, 1.0f});
        ImGui::Text("*** %s ***", achieveMsg.c_str());
        ImGui::PopStyleColor();
    }

    ImGui::SetCursorPos({cx - 200.0f, cy - 90.0f});
    ImGui::PushStyleColor(ImGuiCol_Text, Theme::Gold());
    ImGui::Text("------------------------------------------------------------");
    ImGui::PopStyleColor();

    // Game buttons — two columns
    ImVec2 btnSize{200.0f, 44.0f};
    float  col1 = cx - 210.0f;
    float  col2 = cx + 12.0f;
    float  btnY = cy - 70.0f;
    float  step = 52.0f;

    Theme::PushButtonGold();

    ImGui::SetCursorPos({col1, btnY});
    if (ImGui::Button("  Slot Machine  ", btnSize)) {
        AudioManager::instance().play(Sound::ButtonClick, 60);
        sm.changeState(std::make_unique<SlotMachineState>(sm, app));
    }

    ImGui::SetCursorPos({col2, btnY});
    if (ImGui::Button("  Blackjack     ", btnSize)) {
        AudioManager::instance().play(Sound::ButtonClick, 60);
        sm.changeState(std::make_unique<BlackjackState>(sm, app));
    }

    btnY += step;
    ImGui::SetCursorPos({col1, btnY});
    if (ImGui::Button("  Roulette      ", btnSize)) {
        AudioManager::instance().play(Sound::ButtonClick, 60);
        sm.changeState(std::make_unique<RouletteState>(sm, app));
    }

    ImGui::SetCursorPos({col2, btnY});
    if (ImGui::Button("  Video Poker   ", btnSize)) {
        AudioManager::instance().play(Sound::ButtonClick, 60);
        sm.changeState(std::make_unique<VideoPokerState>(sm, app));
    }

    Theme::PopButtonGold();

    btnY += step;
    ImGui::SetCursorPos({col1, btnY});
    Theme::PushButtonRed();
    if (ImGui::Button("  Russian Roulette  ", btnSize)) {
        AudioManager::instance().play(Sound::ButtonClick, 60);
        sm.changeState(std::make_unique<RussianRouletteState>(sm, app));
    }
    Theme::PopButtonRed();

    ImGui::SetCursorPos({col2, btnY});
    Theme::PushButtonGold();
    if (ImGui::Button("  Highscores    ", btnSize)) {
        AudioManager::instance().play(Sound::ButtonClick, 60);
        sm.pushState(std::make_unique<HighscoreState>(sm, app));
    }

    btnY += step;
    ImGui::SetCursorPos({col1, btnY});
    if (ImGui::Button("  Settings      ", btnSize)) {
        AudioManager::instance().play(Sound::ButtonClick, 60);
        sm.pushState(std::make_unique<SettingsState>(sm, app));
    }
    Theme::PopButtonGold();

    ImGui::SetCursorPos({col2, btnY});
    Theme::PushButtonRed();
    if (ImGui::Button("  Main Menu     ", btnSize)) {
        AudioManager::instance().play(Sound::ButtonClick, 60);
        sm.changeState(std::make_unique<MenuState>(sm, app));
    }
    Theme::PopButtonRed();

    // Achievements summary (bottom line)
    int total = static_cast<int>(Achievement::COUNT);
    int unlocked = 0;
    auto& am = AchievementManager::instance();
    for (int i = 0; i < total; ++i)
        if (am.isUnlocked(static_cast<Achievement>(i))) ++unlocked;

    ImGui::SetCursorPos({cx - 200.0f, io.DisplaySize.y - 35.0f});
    ImGui::PushStyleColor(ImGuiCol_Text, Theme::DarkGold());
    ImGui::Text("Achievements: %d / %d   |   F11: Fullscreen", unlocked, total);
    ImGui::PopStyleColor();

    ImGui::End();
}
