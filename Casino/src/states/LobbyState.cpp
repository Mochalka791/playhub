#include "LobbyState.h"
#include "MenuState.h"
#include "SlotMachineState.h"
#include "BlackjackState.h"
#include "RouletteState.h"
#include "RussianRouletteState.h"
#include "HighscoreState.h"
#include "../core/StateManager.h"
#include "../core/Application.h"
#include "../models/Player.h"
#include "../models/HighscoreManager.h"
#include "../ui/Theme.h"
#include <imgui.h>

LobbyState::LobbyState(StateManager& sm, Application& app)
    : IState(sm, app) {}

void LobbyState::onEnter()
{
    // Submit current player state to highscore whenever returning to lobby
    if (Player* p = app.getPlayer())
        HighscoreManager::instance().submit(*p);
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
    ImGui::SetCursorPos({cx - 60.0f, cy - 200.0f});
    ImGui::Text("LOBBY");
    ImGui::SetWindowFontScale(1.0f);
    Theme::PopTitleStyle();

    // Player info
    if (Player* p = app.getPlayer()) {
        ImGui::SetCursorPos({cx - 200.0f, cy - 150.0f});
        ImGui::Text("Welcome, %s!", p->getName().c_str());
        ImGui::SetCursorPos({cx - 200.0f, cy - 120.0f});
        ImGui::PushStyleColor(ImGuiCol_Text, Theme::Gold());
        ImGui::Text("Balance: $%.2f    Net: %+.2f$    Games: %d",
                    p->getBalance(), p->getNetProfit(), p->getGamesPlayed());
        ImGui::PopStyleColor();
    }

    ImGui::SetCursorPos({cx - 200.0f, cy - 80.0f});
    ImGui::PushStyleColor(ImGuiCol_Text, Theme::Gold());
    ImGui::Text("---------------------------------------------");
    ImGui::PopStyleColor();

    // Game buttons
    ImVec2 btnSize{260.0f, 50.0f};
    float  btnX = cx - 130.0f;

    Theme::PushButtonGold();

    ImGui::SetCursorPos({btnX, cy - 50.0f});
    if (ImGui::Button("  Slot Machine  ", btnSize))
        sm.changeState(std::make_unique<SlotMachineState>(sm, app));

    ImGui::SetCursorPos({btnX, cy + 15.0f});
    if (ImGui::Button("  Blackjack     ", btnSize))
        sm.changeState(std::make_unique<BlackjackState>(sm, app));

    ImGui::SetCursorPos({btnX, cy + 80.0f});
    if (ImGui::Button("  Roulette      ", btnSize))
        sm.changeState(std::make_unique<RouletteState>(sm, app));

    Theme::PopButtonGold();

    // Russian Roulette — red button (dangerous!)
    ImGui::SetCursorPos({btnX, cy + 145.0f});
    Theme::PushButtonRed();
    if (ImGui::Button("  Russian Roulette  ", btnSize))
        sm.changeState(std::make_unique<RussianRouletteState>(sm, app));
    Theme::PopButtonRed();

    ImGui::SetCursorPos({btnX, cy + 210.0f});
    if (ImGui::Button("  Highscores    ", btnSize))
        sm.pushState(std::make_unique<HighscoreState>(sm, app));

    ImGui::SetCursorPos({btnX, cy + 275.0f});
    Theme::PushButtonRed();
    if (ImGui::Button("  Main Menu     ", btnSize))
        sm.changeState(std::make_unique<MenuState>(sm, app));
    Theme::PopButtonRed();

    ImGui::End();
}
