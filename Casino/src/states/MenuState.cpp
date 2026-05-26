#include "MenuState.h"
#include "LoginState.h"
#include "../core/StateManager.h"
#include "../core/Application.h"
#include "../ui/Theme.h"
#include "../audio/AudioManager.h"
#include <imgui.h>

MenuState::MenuState(StateManager& sm, Application& app)
    : IState(sm, app) {}

void MenuState::onEnter() {}

void MenuState::render()
{
    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos({0, 0});
    ImGui::SetNextWindowSize(io.DisplaySize);
    ImGui::Begin("##menu", nullptr,
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoScrollbar  | ImGuiWindowFlags_NoSavedSettings);

    float cx = io.DisplaySize.x * 0.5f;
    float cy = io.DisplaySize.y * 0.5f;

    // Title
    const char* title = "  CASINO  ";
    ImGui::SetCursorPos({cx - 120.0f, cy - 160.0f});
    Theme::PushTitleStyle();
    ImGui::SetWindowFontScale(2.6f);
    ImGui::Text("%s", title);
    ImGui::SetWindowFontScale(1.0f);
    Theme::PopTitleStyle();

    ImGui::SetCursorPos({cx - 120.0f, cy - 100.0f});
    ImGui::PushStyleColor(ImGuiCol_Text, Theme::Gold());
    ImGui::Text("=============================");
    ImGui::PopStyleColor();

    // Subtitle
    ImGui::SetCursorPos({cx - 90.0f, cy - 60.0f});
    ImGui::Text("Slot Machine  |  Blackjack  |  Roulette");

    ImGui::SetCursorPos({cx - 120.0f, cy - 10.0f});
    ImGui::PushStyleColor(ImGuiCol_Text, Theme::Gold());
    ImGui::Text("=============================");
    ImGui::PopStyleColor();

    // Buttons
    ImVec2 btnSize{200.0f, 45.0f};
    Theme::PushButtonGold();

    ImGui::SetCursorPos({cx - 100.0f, cy + 30.0f});
    if (ImGui::Button("  PLAY  ", btnSize)) {
        AudioManager::instance().play(Sound::ButtonClick, 80);
        sm.changeState(std::make_unique<LoginState>(sm, app));
    }

    Theme::PopButtonGold();
    Theme::PushButtonRed();

    ImGui::SetCursorPos({cx - 100.0f, cy + 95.0f});
    if (ImGui::Button("  EXIT  ", btnSize)) {
        AudioManager::instance().play(Sound::ButtonClick, 60);
        app.stop();
    }

    Theme::PopButtonRed();
    ImGui::End();
}
