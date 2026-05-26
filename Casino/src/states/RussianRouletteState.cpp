#include "RussianRouletteState.h"
#include "LobbyState.h"
#include "../core/StateManager.h"
#include "../core/Application.h"
#include "../models/Player.h"
#include "../ui/Theme.h"
#include <imgui.h>
#include <cmath>
#include <string>

static constexpr float M_PI_F = 3.14159265f;
static constexpr float PULL_DURATION = 1.2f;

RussianRouletteState::RussianRouletteState(StateManager& sm, Application& app)
    : IState(sm, app) {}

void RussianRouletteState::onEnter()
{
    game = std::make_unique<RussianRoulette>(*app.getPlayer());
    phase        = RRPhase::Setup;
    betAmount    = 10.0f;
    pullTimer    = 0.0f;
    shakeTimer   = 0.0f;
    cylinderAngle = 0.0f;
    resultMsg.clear();
}

void RussianRouletteState::update(float dt)
{
    if (phase == RRPhase::Pulling) {
        cylinderAngle += dt * 18.0f; // fast spin
        pullTimer += dt;
        if (pullTimer >= PULL_DURATION) {
            phase     = RRPhase::Result;
            shakeTimer = game->wasShot() ? 0.5f : 0.0f;

            if (game->wasShot())
                resultMsg = "BANG!  You were shot.  -$" + std::to_string((int)game->getBet());
            else
                resultMsg = "Click...  Safe!   +" + std::to_string((int)(game->getPayout() - game->getBet())) + "$";
        }
    }

    if (shakeTimer > 0.0f)
        shakeTimer -= dt;
}

void RussianRouletteState::render()
{
    ImGuiIO& io = ImGui::GetIO();

    // Screen shake when shot
    float shakeX = 0.0f, shakeY = 0.0f;
    if (shakeTimer > 0.0f) {
        float s = shakeTimer * 12.0f;
        shakeX = (((int)(shakeTimer * 60) % 5) - 2) * s;
        shakeY = (((int)(shakeTimer * 47) % 5) - 2) * s * 0.5f;
    }

    ImGui::SetNextWindowPos({shakeX, shakeY});
    ImGui::SetNextWindowSize(io.DisplaySize);

    // Red flash when shot
    bool shot = (phase == RRPhase::Result && game->wasShot());
    if (shot) {
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4{0.35f, 0.02f, 0.02f, 1.0f});
    }

    ImGui::Begin("##rr", nullptr,
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoScrollbar  | ImGuiWindowFlags_NoSavedSettings);

    if (shot) ImGui::PopStyleColor();

    float cx = io.DisplaySize.x * 0.5f;
    float cy = io.DisplaySize.y * 0.5f;

    // Title
    Theme::PushTitleStyle();
    ImGui::SetWindowFontScale(1.8f);
    ImGui::SetCursorPos({cx - 160.0f, 30.0f});
    ImGui::PushStyleColor(ImGuiCol_Text, Theme::Red());
    ImGui::Text("RUSSIAN ROULETTE");
    ImGui::PopStyleColor();
    ImGui::SetWindowFontScale(1.0f);
    Theme::PopTitleStyle();

    // Balance
    if (Player* p = app.getPlayer()) {
        ImGui::SetCursorPos({cx - 160.0f, 80.0f});
        ImGui::PushStyleColor(ImGuiCol_Text, Theme::Gold());
        ImGui::Text("Balance: $%.2f", p->getBalance());
        ImGui::PopStyleColor();
    }

    // Cylinder always visible
    ImVec2 winPos = ImGui::GetWindowPos();
    renderCylinder(winPos.x + cx - 100.0f, winPos.y + cy - 60.0f);

    if (phase == RRPhase::Setup || phase == RRPhase::Pulling)
        renderSetup(cx, cy);
    else
        renderResult(cx, cy);

    ImGui::End();
}

void RussianRouletteState::renderCylinder(float screenX, float screenY)
{
    ImDrawList* draw = ImGui::GetWindowDrawList();
    ImVec2 center = {screenX, screenY};
    float outerR = 90.0f;
    float chamberR = 20.0f;

    // Outer casing
    draw->AddCircleFilled(center, outerR + 10.0f, IM_COL32(30, 22, 10, 255), 64);
    draw->AddCircle(center, outerR + 10.0f, IM_COL32(150, 120, 50, 255), 64, 2.5f);
    draw->AddCircle(center, outerR + 5.0f,  IM_COL32(100, 80, 30, 255),  64, 1.0f);

    float spinOffset = (phase == RRPhase::Pulling) ? cylinderAngle : 0.0f;

    for (int i = 0; i < 6; ++i) {
        float angle = spinOffset + i * (2.0f * M_PI_F / 6.0f);
        ImVec2 pos = {
            center.x + cosf(angle) * outerR * 0.55f,
            center.y + sinf(angle) * outerR * 0.55f
        };

        bool hasBullet  = false;
        bool isFiredCh  = false;
        bool wasShotRes = false;

        if (phase == RRPhase::Result) {
            hasBullet  = game->getChambers()[i];
            isFiredCh  = (i == game->getFiredChamber());
            wasShotRes = game->wasShot();
        }

        // Chamber background
        ImU32 fillCol;
        if (isFiredCh && wasShotRes)
            fillCol = IM_COL32(220, 30, 30, 255);   // fired bullet: red
        else if (isFiredCh && !wasShotRes)
            fillCol = IM_COL32(30, 200, 60, 255);   // survived: green
        else if (hasBullet)
            fillCol = IM_COL32(50, 40, 20, 255);    // loaded bullet: dark gold
        else
            fillCol = IM_COL32(18, 18, 18, 255);    // empty: near black

        draw->AddCircleFilled(pos, chamberR, fillCol, 32);
        draw->AddCircle(pos, chamberR, IM_COL32(140, 110, 45, 255), 32, 1.5f);

        // Bullet indicator dot inside loaded chamber
        if (hasBullet && !(isFiredCh && wasShotRes)) {
            draw->AddCircleFilled(pos, chamberR * 0.45f, IM_COL32(200, 170, 60, 255), 16);
            draw->AddCircle(pos, chamberR * 0.45f, IM_COL32(255, 220, 80, 255), 16, 1.0f);
        }

        // Smoke/flash from fired chamber
        if (isFiredCh && wasShotRes) {
            draw->AddCircleFilled(pos, chamberR * 0.3f, IM_COL32(255, 200, 60, 220), 12);
        }
    }

    // Barrel hole (center)
    draw->AddCircleFilled(center, 14.0f, IM_COL32(4, 4, 4, 255), 24);
    draw->AddCircle(center, 14.0f, IM_COL32(120, 95, 40, 255), 24, 2.0f);
    draw->AddCircle(center, 8.0f,  IM_COL32(40, 30, 10, 255),  24, 1.0f);
}

void RussianRouletteState::renderSetup(float cx, float cy)
{
    float panelX = cx + 30.0f;
    float panelY = cy - 140.0f;

    // Bullet count
    ImGui::SetCursorPos({panelX, panelY});
    ImGui::Text("Bullets loaded:");
    ImGui::SetCursorPos({panelX, panelY + 28.0f});
    ImGui::SetNextItemWidth(220.0f);

    int bullets = game->getBullets();
    if (ImGui::SliderInt("##bullets", &bullets, 1, 5)) {
        game->setBullets(bullets);
    }

    // Risk + multiplier info
    float mult = game->getMultiplier();
    float surviveChance = (6 - bullets) / 6.0f * 100.0f;

    ImGui::SetCursorPos({panelX, panelY + 65.0f});
    ImVec4 riskColor = (bullets <= 2) ? Theme::Green()
                     : (bullets == 3) ? Theme::Gold()
                     : Theme::Red();
    ImGui::PushStyleColor(ImGuiCol_Text, riskColor);
    ImGui::Text("Survive chance: %.0f%%    Multiplier: %.1fx", surviveChance, mult);
    ImGui::PopStyleColor();

    // Bet
    ImGui::SetCursorPos({panelX, panelY + 100.0f});
    ImGui::Text("Bet Amount:");
    ImGui::SetCursorPos({panelX, panelY + 125.0f});
    ImGui::SetNextItemWidth(220.0f);
    float maxBet = app.getPlayer() ? (float)app.getPlayer()->getBalance() : 100.0f;
    ImGui::SliderFloat("##rrbet", &betAmount, 1.0f, std::max(1.0f, maxBet), "%.0f $");

    // Expected payout
    ImGui::SetCursorPos({panelX, panelY + 165.0f});
    ImGui::PushStyleColor(ImGuiCol_Text, Theme::Gold());
    ImGui::Text("If survive: +$%.0f   If shot: -$%.0f", betAmount * mult - betAmount, betAmount);
    ImGui::PopStyleColor();

    // Pull Trigger button
    bool canPull = (phase == RRPhase::Setup)
                && app.getPlayer()
                && app.getPlayer()->canBet(betAmount);

    ImGui::SetCursorPos({panelX, panelY + 205.0f});
    ImGui::PushStyleColor(ImGuiCol_Button,        {0.45f, 0.02f, 0.02f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, {0.65f, 0.04f, 0.04f, 1.0f});
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  {0.30f, 0.01f, 0.01f, 1.0f});

    if (!canPull) ImGui::BeginDisabled();
    if (phase == RRPhase::Pulling) {
        // Show pulling animation dots
        int d = (int)(pullTimer * 4) % 4;
        const char* dots[] = {".", "..", "...", "...."};
        ImGui::Button((std::string("  Pulling") + dots[d] + "  ").c_str(), {220.0f, 50.0f});
    } else {
        if (ImGui::Button("  PULL TRIGGER  ", {220.0f, 50.0f})) {
            game->placeBet(betAmount);
            game->play();
            phase     = RRPhase::Pulling;
            pullTimer = 0.0f;
            cylinderAngle = 0.0f;
        }
    }
    if (!canPull) ImGui::EndDisabled();
    ImGui::PopStyleColor(3);

    // Back button
    ImGui::SetCursorPos({panelX, panelY + 265.0f});
    if (ImGui::Button("  Back to Lobby  ", {220.0f, 38.0f}))
        sm.changeState(std::make_unique<LobbyState>(sm, app));
}

void RussianRouletteState::renderResult(float cx, float cy)
{
    bool survived = !game->wasShot();
    float panelX = cx + 30.0f;
    float panelY = cy - 80.0f;

    // Big result text
    ImGui::SetCursorPos({panelX, panelY});
    ImGui::SetWindowFontScale(2.0f);
    ImGui::PushStyleColor(ImGuiCol_Text, survived ? Theme::Green() : Theme::Red());
    ImGui::Text("%s", survived ? "CLICK..." : "BANG!!!");
    ImGui::PopStyleColor();
    ImGui::SetWindowFontScale(1.0f);

    ImGui::SetCursorPos({panelX, panelY + 60.0f});
    ImGui::PushStyleColor(ImGuiCol_Text, survived ? Theme::Gold() : Theme::Red());
    ImGui::Text("%s", resultMsg.c_str());
    ImGui::PopStyleColor();

    // Bullet indicators explanation
    ImGui::SetCursorPos({panelX, panelY + 100.0f});
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{0.6f, 0.6f, 0.6f, 1.0f});
    ImGui::Text("Green = survived chamber   Red = bullet fired   Gold = loaded");
    ImGui::PopStyleColor();

    ImVec2 btnSize{200.0f, 42.0f};

    Theme::PushButtonGold();
    ImGui::SetCursorPos({panelX, panelY + 145.0f});
    if (ImGui::Button("  Play Again  ", btnSize)) {
        game = std::make_unique<RussianRoulette>(*app.getPlayer());
        phase = RRPhase::Setup;
        resultMsg.clear();
        pullTimer = 0.0f;
        shakeTimer = 0.0f;
        cylinderAngle = 0.0f;
    }
    Theme::PopButtonGold();

    ImGui::SetCursorPos({panelX, panelY + 198.0f});
    if (ImGui::Button("  Back to Lobby  ", btnSize))
        sm.changeState(std::make_unique<LobbyState>(sm, app));
}
