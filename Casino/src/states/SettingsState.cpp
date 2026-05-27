#include "SettingsState.h"
#include "../core/StateManager.h"
#include "../core/Application.h"
#include "../core/AppSettings.h"
#include "../ui/Theme.h"
#include "../audio/AudioManager.h"
#include <imgui.h>
#include <SDL2/SDL.h>

SettingsState::SettingsState(StateManager& sm, Application& app)
    : IState(sm, app) {}

void SettingsState::onEnter()
{
    auto& s    = AppSettings::instance();
    volume         = s.masterVolume;
    fullscreen     = s.fullscreen;
    dealerStrategy = s.dealerStrategy;
    minBet         = s.minBet;
    maxBet         = s.maxBet;
}

void SettingsState::render()
{
    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos({0, 0});
    ImGui::SetNextWindowSize(io.DisplaySize);
    ImGui::Begin("##settings", nullptr,
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoScrollbar  | ImGuiWindowFlags_NoSavedSettings);

    float cx  = io.DisplaySize.x * 0.5f;
    float top = 40.0f;
    float lx  = cx - 200.0f;

    Theme::PushTitleStyle();
    ImGui::SetWindowFontScale(1.8f);
    ImGui::SetCursorPos({cx - 80.0f, top});
    ImGui::Text("SETTINGS");
    ImGui::SetWindowFontScale(1.0f);
    Theme::PopTitleStyle();

    float y = top + 70.0f;

    // ---- Audio ----
    ImGui::SetCursorPos({lx, y});
    ImGui::PushStyleColor(ImGuiCol_Text, Theme::Gold());
    ImGui::Text("--- Audio ---");
    ImGui::PopStyleColor();
    y += 28.0f;

    ImGui::SetCursorPos({lx, y});
    ImGui::Text("Master Volume:");
    ImGui::SetCursorPos({lx + 160.0f, y});
    ImGui::SetNextItemWidth(260.0f);
    if (ImGui::SliderInt("##vol", &volume, 0, 128)) {
        AudioManager::instance().setMasterVolume(volume);
    }
    y += 36.0f;

    // ---- Display ----
    ImGui::SetCursorPos({lx, y});
    ImGui::PushStyleColor(ImGuiCol_Text, Theme::Gold());
    ImGui::Text("--- Display ---");
    ImGui::PopStyleColor();
    y += 28.0f;

    ImGui::SetCursorPos({lx, y});
    ImGui::Text("Fullscreen (F11):");
    ImGui::SetCursorPos({lx + 180.0f, y});
    if (ImGui::Checkbox("##fs", &fullscreen)) {
        app.setFullscreen(fullscreen);
    }
    y += 36.0f;

    // ---- Blackjack ----
    ImGui::SetCursorPos({lx, y});
    ImGui::PushStyleColor(ImGuiCol_Text, Theme::Gold());
    ImGui::Text("--- Blackjack ---");
    ImGui::PopStyleColor();
    y += 28.0f;

    ImGui::SetCursorPos({lx, y});
    ImGui::Text("Dealer Strategy:");
    ImGui::SetCursorPos({lx + 160.0f, y});
    ImGui::SetNextItemWidth(260.0f);
    const char* strats[] = {"Hit on Soft 17 (Standard)", "Stand on Soft 17 (Strict)"};
    ImGui::Combo("##strat", &dealerStrategy, strats, 2);
    y += 36.0f;

    // ---- Table Limits ----
    ImGui::SetCursorPos({lx, y});
    ImGui::PushStyleColor(ImGuiCol_Text, Theme::Gold());
    ImGui::Text("--- Table Limits ---");
    ImGui::PopStyleColor();
    y += 28.0f;

    ImGui::SetCursorPos({lx, y});
    ImGui::Text("Minimum Bet ($):");
    ImGui::SetCursorPos({lx + 160.0f, y});
    ImGui::SetNextItemWidth(260.0f);
    ImGui::SliderFloat("##minb", &minBet, 1.0f, 500.0f, "%.0f");
    if (minBet > maxBet) maxBet = minBet;
    y += 36.0f;

    ImGui::SetCursorPos({lx, y});
    ImGui::Text("Maximum Bet ($):");
    ImGui::SetCursorPos({lx + 160.0f, y});
    ImGui::SetNextItemWidth(260.0f);
    ImGui::SliderFloat("##maxb", &maxBet, minBet, 10000.0f, "%.0f");
    y += 50.0f;

    // ---- Buttons ----
    ImVec2 btnSize{160.0f, 42.0f};
    Theme::PushButtonGold();
    ImGui::SetCursorPos({lx, y});
    if (ImGui::Button("  Save & Back  ", btnSize)) {
        auto& s          = AppSettings::instance();
        s.masterVolume   = volume;
        s.fullscreen     = fullscreen;
        s.dealerStrategy = dealerStrategy;
        s.minBet         = minBet;
        s.maxBet         = maxBet;
        s.save();
        AudioManager::instance().setMasterVolume(volume);
        app.setFullscreen(fullscreen);
        sm.popState();
    }
    Theme::PopButtonGold();

    ImGui::SetCursorPos({lx + 180.0f, y});
    if (ImGui::Button("  Cancel  ", btnSize))
        sm.popState();

    ImGui::End();
}
