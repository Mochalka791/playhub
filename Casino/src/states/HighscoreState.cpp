#include "HighscoreState.h"
#include "../core/StateManager.h"
#include "../core/Application.h"
#include "../models/HighscoreManager.h"
#include "../ui/Theme.h"
#include <imgui.h>

HighscoreState::HighscoreState(StateManager& sm, Application& app)
    : IState(sm, app) {}

void HighscoreState::render()
{
    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos({0, 0});
    ImGui::SetNextWindowSize(io.DisplaySize);
    ImGui::Begin("##highscore", nullptr,
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoScrollbar  | ImGuiWindowFlags_NoSavedSettings);

    float cx = io.DisplaySize.x * 0.5f;
    float top = 60.0f;

    Theme::PushTitleStyle();
    ImGui::SetWindowFontScale(1.8f);
    ImGui::SetCursorPos({cx - 100.0f, top});
    ImGui::Text("HIGHSCORES");
    ImGui::SetWindowFontScale(1.0f);
    Theme::PopTitleStyle();

    ImGui::SetCursorPos({cx - 250.0f, top + 55.0f});
    ImGui::PushStyleColor(ImGuiCol_Text, Theme::Gold());
    ImGui::Text("%-4s %-20s %12s %12s %8s",
                "#", "Name", "Net Profit", "Balance", "Games");
    ImGui::PopStyleColor();

    ImGui::SetCursorPos({cx - 250.0f, top + 75.0f});
    ImGui::PushStyleColor(ImGuiCol_Text, Theme::Gold());
    ImGui::Text("------------------------------------------------------------");
    ImGui::PopStyleColor();

    const auto& entries = HighscoreManager::instance().getEntries();
    float y = top + 100.0f;
    int   rank = 1;
    for (const auto& e : entries) {
        ImGui::SetCursorPos({cx - 250.0f, y});
        ImVec4 col = (e.netProfit >= 0) ? Theme::Green() : Theme::Red();
        ImGui::PushStyleColor(ImGuiCol_Text, col);
        ImGui::Text("%-4d %-20s %+12.2f $ %12.2f $ %8d",
                    rank, e.name.c_str(), e.netProfit, e.balance, e.gamesPlayed);
        ImGui::PopStyleColor();
        y += 28.0f;
        ++rank;
    }

    if (entries.empty()) {
        ImGui::SetCursorPos({cx - 100.0f, y + 20.0f});
        ImGui::Text("No entries yet.");
    }

    // Back button
    ImGui::SetCursorPos({cx - 80.0f, io.DisplaySize.y - 70.0f});
    if (ImGui::Button("  Back  ", {160.0f, 40.0f}))
        sm.popState();

    ImGui::End();
}
