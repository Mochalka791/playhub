#include "LoginState.h"
#include "LobbyState.h"
#include "MenuState.h"
#include "../core/StateManager.h"
#include "../core/Application.h"
#include "../models/Player.h"
#include "../ui/Theme.h"
#include <imgui.h>
#include <cstring>

LoginState::LoginState(StateManager& sm, Application& app)
    : IState(sm, app)
{}

void LoginState::onEnter()
{
    nameBuf.fill(0);
    startBalance = 1000.0f;
    errorMsg.clear();
}

void LoginState::render()
{
    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos({0, 0});
    ImGui::SetNextWindowSize(io.DisplaySize);
    ImGui::Begin("##login", nullptr,
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoScrollbar  | ImGuiWindowFlags_NoSavedSettings);

    float cx = io.DisplaySize.x * 0.5f;
    float cy = io.DisplaySize.y * 0.5f;

    Theme::PushTitleStyle();
    ImGui::SetWindowFontScale(1.8f);
    ImGui::SetCursorPos({cx - 90.0f, cy - 160.0f});
    ImGui::Text("Player Setup");
    ImGui::SetWindowFontScale(1.0f);
    Theme::PopTitleStyle();

    ImGui::SetCursorPos({cx - 150.0f, cy - 90.0f});
    ImGui::Text("Name:");
    ImGui::SetCursorPos({cx - 150.0f, cy - 65.0f});
    ImGui::SetNextItemWidth(300.0f);
    ImGui::InputText("##name", nameBuf.data(), nameBuf.size());

    ImGui::SetCursorPos({cx - 150.0f, cy - 20.0f});
    ImGui::Text("Starting Balance ($):");
    ImGui::SetCursorPos({cx - 150.0f, cy + 5.0f});
    ImGui::SetNextItemWidth(300.0f);
    ImGui::SliderFloat("##balance", &startBalance, 100.0f, 10000.0f, "%.0f $");

    if (!errorMsg.empty()) {
        ImGui::SetCursorPos({cx - 150.0f, cy + 50.0f});
        ImGui::PushStyleColor(ImGuiCol_Text, Theme::Red());
        ImGui::Text("%s", errorMsg.c_str());
        ImGui::PopStyleColor();
    }

    ImVec2 btnSize{140.0f, 38.0f};
    Theme::PushButtonGold();
    ImGui::SetCursorPos({cx - 150.0f, cy + 85.0f});
    if (ImGui::Button("  START  ", btnSize)) {
        if (std::strlen(nameBuf.data()) == 0) {
            errorMsg = "Please enter your name.";
        } else if (startBalance < 100.0f) {
            errorMsg = "Minimum balance is $100.";
        } else {
            app.setPlayer(std::make_unique<Player>(nameBuf.data(), startBalance));
            sm.changeState(std::make_unique<LobbyState>(sm, app));
        }
    }
    Theme::PopButtonGold();

    ImGui::SetCursorPos({cx + 10.0f, cy + 85.0f});
    if (ImGui::Button("  Back  ", btnSize)) {
        sm.changeState(std::make_unique<MenuState>(sm, app));
    }

    ImGui::End();
}
