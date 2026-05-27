#include "RouletteState.h"
#include "LobbyState.h"
#include "../core/StateManager.h"
#include "../core/Application.h"
#include "../core/AppSettings.h"
#include "../models/Player.h"
#include "../models/AchievementManager.h"
#include "../ui/Theme.h"
#include "../audio/AudioManager.h"
#include <imgui.h>
#include <cmath>
#include <string>
#include <algorithm>

static constexpr float PI_F = 3.14159265f;

// European roulette wheel order (official layout)
static constexpr int WHEEL_ORDER[] = {
    0,32,15,19,4,21,2,25,17,34,6,27,13,36,11,30,8,23,10,
    5,24,16,33,1,20,14,31,9,22,18,29,7,28,12,35,3,26
};
static constexpr int WHEEL_COUNT = 37;

RouletteState::RouletteState(StateManager& sm, Application& app)
    : IState(sm, app) {}

void RouletteState::onEnter()
{
    game = std::make_unique<Roulette>(*app.getPlayer());
    betAmount   = 10.0f;
    straightNum = 0;
    resultMsg.clear();
    showResult  = false;
    spinAnim    = {};
    wheelAngle  = 0.0f;
    finalNumber = -1;
    spinSoundPlayed   = false;
    resultSoundPlayed = false;
    particles.clear();
}

int RouletteState::findWheelIndex(int number) const
{
    for (int i = 0; i < WHEEL_COUNT; ++i)
        if (WHEEL_ORDER[i] == number) return i;
    return 0;
}

void RouletteState::update(float dt)
{
    particles.update(dt);

    if (spinAnim.active) {
        if (!spinSoundPlayed) {
            spinSoundPlayed = true;
            AudioManager::instance().play(Sound::WheelSpin, 90);
            AudioManager::instance().play(Sound::BallRoll, 80);
        }

        spinAnim.update(dt);
        float t = spinAnim.progress();

        // Ease-out: fast start, decelerate to final
        float eased = 1.0f - powf(1.0f - t, 2.5f);
        wheelAngle = spinStartAngle + eased * totalSpinAmt;

        if (spinAnim.finished()) {
            wheelAngle  = spinStartAngle + totalSpinAmt;
            showResult  = true;
        }
    }

    if (showResult && !resultSoundPlayed) {
        resultSoundPlayed = true;
        ImGuiIO& io = ImGui::GetIO();
        float cx = io.DisplaySize.x * 0.5f;
        float cy = io.DisplaySize.y * 0.5f;
        bool won = (game->getResult() == GameResult::Win);
        if (won) {
            AudioManager::instance().play(Sound::WinFanfare, 115);
            particles.emit(cx, cy, 60, ParticleType::Coin);
            particles.emit(cx, cy, 40, ParticleType::Confetti);
        } else {
            AudioManager::instance().play(Sound::Lose, 90);
        }
    }
}

// ---- Wheel rendering ----------------------------------------

void RouletteState::renderWheel(float screenX, float screenY, float radius)
{
    ImDrawList* draw = ImGui::GetWindowDrawList();
    ImVec2 center    = {screenX, screenY};

    float segAngle = 2.0f * PI_F / WHEEL_COUNT;

    // Outer decorative ring
    draw->AddCircleFilled(center, radius + 14.0f, IM_COL32(25, 18, 5,  255), 80);
    draw->AddCircle(center, radius + 14.0f, IM_COL32(180, 145, 40, 255), 80, 3.0f);
    draw->AddCircle(center, radius + 10.0f, IM_COL32(120, 95,  25, 255), 80, 1.0f);

    // Draw 37 coloured sectors
    for (int i = 0; i < WHEEL_COUNT; ++i) {
        int   num = WHEEL_ORDER[i];
        float a1  = wheelAngle + i * segAngle - segAngle * 0.5f;
        float a2  = a1 + segAngle;

        ImU32 col;
        if (num == 0)
            col = IM_COL32(0, 145, 30, 255);
        else if (game->isRedNumber(num))
            col = IM_COL32(185, 20, 20, 255);
        else
            col = IM_COL32(14, 14, 14, 255);

        // Sector triangle fan
        const int STEPS = 5;
        ImVec2 pts[STEPS + 2];
        pts[0] = center;
        for (int s = 0; s <= STEPS; ++s) {
            float a = a1 + (a2 - a1) * s / STEPS;
            pts[s + 1] = {center.x + cosf(a) * radius, center.y + sinf(a) * radius};
        }
        draw->AddConvexPolyFilled(pts, STEPS + 2, col);

        // Thin border between sectors
        draw->AddLine(pts[0], pts[1], IM_COL32(60, 45, 5, 200), 1.0f);
    }

    // Number labels on the rim
    for (int i = 0; i < WHEEL_COUNT; ++i) {
        int   num   = WHEEL_ORDER[i];
        float angle = wheelAngle + i * segAngle;
        float labelR = radius * 0.82f;
        ImVec2 lPos = {center.x + cosf(angle) * labelR,
                       center.y + sinf(angle) * labelR};

        ImU32 textCol = (num == 0) ? IM_COL32(255, 255, 255, 255)
                                   : IM_COL32(255, 240, 200, 255);
        char buf[4];
        snprintf(buf, sizeof(buf), "%d", num);
        draw->AddText({lPos.x - 5.0f, lPos.y - 6.0f}, textCol, buf);
    }

    // Outer gold rim
    draw->AddCircle(center, radius, IM_COL32(200, 165, 45, 255), 80, 2.5f);

    // Inner felt area
    draw->AddCircleFilled(center, radius * 0.28f, IM_COL32(6, 60, 10, 255), 48);
    draw->AddCircle(center, radius * 0.28f, IM_COL32(160, 130, 40, 255), 48, 2.0f);

    // Ball: during spin = orbiting, stopped = at winning sector
    float ballR = radius * 0.88f;
    float ballAngle;
    if (spinAnim.active) {
        // Ball orbits in the opposite direction slightly faster
        ballAngle = -wheelAngle * 1.15f;
    } else if (finalNumber >= 0) {
        int idx = findWheelIndex(finalNumber);
        ballAngle = wheelAngle + idx * segAngle;
    } else {
        ballAngle = -PI_F * 0.5f;
    }
    ImVec2 ballPos = {center.x + cosf(ballAngle) * ballR,
                      center.y + sinf(ballAngle) * ballR};
    draw->AddCircleFilled(ballPos, 8.0f,  IM_COL32(235, 235, 235, 255), 16);
    draw->AddCircle(ballPos,       8.0f,  IM_COL32(255, 255, 255, 255), 16, 1.5f);

    // Top pointer triangle
    ImVec2 pTip = {center.x, center.y - radius - 14.0f};
    ImVec2 pL   = {center.x - 7.0f, center.y - radius + 2.0f};
    ImVec2 pR   = {center.x + 7.0f, center.y - radius + 2.0f};
    draw->AddTriangleFilled(pTip, pL, pR, IM_COL32(255, 220, 30, 255));
    draw->AddTriangle(pTip, pL, pR, IM_COL32(180, 140, 10, 255), 1.5f);
}

// ---- Visual betting table ------------------------------------
static const int RED_NUMS[] = {1,3,5,7,9,12,14,16,18,19,21,23,25,27,30,32,34,36};

static bool isRed(int n) {
    for (int r : RED_NUMS) if (r == n) return true;
    return false;
}

void RouletteState::renderBettingTable(float tableX, float tableY)
{
    ImDrawList* draw = ImGui::GetWindowDrawList();
    ImVec2 winPos = ImGui::GetWindowPos();
    float wx = winPos.x + tableX;
    float wy = winPos.y + tableY;

    const float CW = 28.0f, CH = 24.0f;
    const float gap = 2.0f;
    const float totalGridW = 12 * (CW + gap);
    bool idle = !spinAnim.active && !showResult;

    // 0 cell (left column, full grid height)
    {
        ImVec2 tl = {wx, wy};
        ImVec2 br = {wx + CW, wy + 3 * (CH + gap)};
        draw->AddRectFilled(tl, br, IM_COL32(0, 130, 30, 255), 3.0f);
        draw->AddRect(tl, br, IM_COL32(180, 160, 60, 200), 3.0f, 0, 1.0f);
        draw->AddText({tl.x + 8, tl.y + 3*(CH+gap)/2 - 7}, IM_COL32(255,255,255,255), "0");
        if (idle) {
            ImGui::SetCursorPos({tableX, tableY});
            if (ImGui::InvisibleButton("##t0", {CW, 3*(CH+gap)})) {
                game->addBet(BetType::Straight, betAmount, 0);
                AudioManager::instance().play(Sound::Chip, 60);
            }
        }
    }

    // Number grid 1-36: columns 1-12, rows 3→1 top to bottom
    for (int col = 1; col <= 12; ++col) {
        for (int row = 3; row >= 1; --row) {
            int num = (col - 1) * 3 + row;
            float cx2 = wx + CW + gap + (col - 1) * (CW + gap);
            float cy2 = wy + (3 - row) * (CH + gap);
            ImVec2 tl = {cx2, cy2};
            ImVec2 br = {cx2 + CW, cy2 + CH};
            ImU32 bg = isRed(num) ? IM_COL32(180, 20, 20, 255) : IM_COL32(15, 15, 15, 255);
            draw->AddRectFilled(tl, br, bg, 2.0f);
            draw->AddRect(tl, br, IM_COL32(140, 120, 50, 180), 2.0f, 0, 1.0f);
            char buf[4]; snprintf(buf, sizeof(buf), "%d", num);
            draw->AddText({tl.x + 4, tl.y + 5}, IM_COL32(255, 255, 255, 240), buf);
            if (idle) {
                ImGui::SetCursorPos({tableX + CW + gap + (col-1)*(CW+gap), tableY + (3-row)*(CH+gap)});
                std::string bid = "##tb" + std::to_string(num);
                if (ImGui::InvisibleButton(bid.c_str(), {CW, CH})) {
                    game->addBet(BetType::Straight, betAmount, num);
                    AudioManager::instance().play(Sound::Chip, 60);
                }
            }
        }
    }

    // 2:1 column bets row
    float below     = wy + 3*(CH+gap) + 4.0f;
    float below_rel = tableY + 3*(CH+gap) + 4.0f;
    float colW = totalGridW / 3.0f - gap;
    BetType colTypes[] = {BetType::Dozen1, BetType::Dozen2, BetType::Dozen3};
    for (int i = 0; i < 3; ++i) {
        float cx2 = wx + CW + gap + i * (colW + gap);
        ImVec2 tl = {cx2, below};
        ImVec2 br = {cx2 + colW, below + CH};
        draw->AddRectFilled(tl, br, IM_COL32(20, 60, 20, 255), 3.0f);
        draw->AddRect(tl, br, IM_COL32(140,120,50,180), 3.0f, 0, 1.0f);
        draw->AddText({cx2 + colW/2 - 8, below + 5}, IM_COL32(200,180,60,255), "2:1");
        if (idle) {
            ImGui::SetCursorPos({tableX + CW + gap + i*(colW+gap), below_rel});
            std::string bid = "##col" + std::to_string(i);
            if (ImGui::InvisibleButton(bid.c_str(), {colW, CH})) {
                game->addBet(colTypes[i], betAmount, -1);
                AudioManager::instance().play(Sound::Chip, 60);
            }
        }
    }

    // Dozen bets row
    float below2     = below + CH + 4.0f;
    float below2_rel = below_rel + CH + 4.0f;
    float dozW = totalGridW / 3.0f - gap;
    const char* doz[] = {"Dozen 1-12","Dozen 13-24","Dozen 25-36"};
    BetType dozT[] = {BetType::Dozen1, BetType::Dozen2, BetType::Dozen3};
    for (int i = 0; i < 3; ++i) {
        float cx2 = wx + CW + gap + i * (dozW + gap);
        ImVec2 tl = {cx2, below2};
        ImVec2 br = {cx2 + dozW, below2 + CH};
        draw->AddRectFilled(tl, br, IM_COL32(25, 50, 25, 255), 3.0f);
        draw->AddRect(tl, br, IM_COL32(140,120,50,180), 3.0f, 0, 1.0f);
        draw->AddText({cx2 + 2, below2 + 5}, IM_COL32(200,200,200,255), doz[i]);
        if (idle) {
            ImGui::SetCursorPos({tableX + CW + gap + i*(dozW+gap), below2_rel});
            std::string bid = "##doz" + std::to_string(i);
            if (ImGui::InvisibleButton(bid.c_str(), {dozW, CH})) {
                game->addBet(dozT[i], betAmount, -1);
                AudioManager::instance().play(Sound::Chip, 60);
            }
        }
    }

    // Outside bets row: 1-18, Even, Red, Black, Odd, 19-36
    float below3     = below2 + CH + 4.0f;
    float below3_rel = below2_rel + CH + 4.0f;
    float outerW = totalGridW / 6.0f - gap;
    const char* outerLabels[] = {"1-18","EVEN","RED","BLACK","ODD","19-36"};
    BetType outerTypes[] = {BetType::Low, BetType::Even, BetType::Red,
                            BetType::Black, BetType::Odd, BetType::High};
    ImU32 outerCols[] = {
        IM_COL32(25,50,25,255), IM_COL32(25,50,25,255),
        IM_COL32(160,15,15,255), IM_COL32(15,15,15,255),
        IM_COL32(25,50,25,255), IM_COL32(25,50,25,255)
    };
    for (int i = 0; i < 6; ++i) {
        float cx2 = wx + CW + gap + i * (outerW + gap);
        ImVec2 tl = {cx2, below3};
        ImVec2 br = {cx2 + outerW, below3 + CH};
        draw->AddRectFilled(tl, br, outerCols[i], 3.0f);
        draw->AddRect(tl, br, IM_COL32(140,120,50,180), 3.0f, 0, 1.0f);
        draw->AddText({cx2 + 2, below3 + 5}, IM_COL32(255,255,255,255), outerLabels[i]);
        if (idle) {
            ImGui::SetCursorPos({tableX + CW + gap + i*(outerW+gap), below3_rel});
            std::string bid = "##out" + std::to_string(i);
            if (ImGui::InvisibleButton(bid.c_str(), {outerW, CH})) {
                game->addBet(outerTypes[i], betAmount, -1);
                AudioManager::instance().play(Sound::Chip, 60);
            }
        }
    }

    // ---- Controls below the grid ----
    float ctrlY = below3_rel + CH + 12.0f;

    // Bet amount slider (disabled while spinning or showing result)
    float maxBet = app.getPlayer() ? (float)app.getPlayer()->getBalance() : 100.0f;
    if (!idle) ImGui::BeginDisabled();
    ImGui::SetCursorPos({tableX, ctrlY});
    ImGui::SetNextItemWidth(totalGridW + CW + gap);
    ImGui::SliderFloat("##betamt", &betAmount, 1.0f, std::max(1.0f, maxBet), "Bet: $%.0f");
    ctrlY += 32.0f;

    // Bets summary + Clear button
    ImGui::SetCursorPos({tableX, ctrlY});
    ImGui::PushStyleColor(ImGuiCol_Text, Theme::Gold());
    ImGui::Text("Total: $%.0f", game->getTotalBetAmount());
    ImGui::PopStyleColor();
    ImGui::SameLine(0.0f, 16.0f);
    if (ImGui::Button("Clear##cbet", {55.0f, 20.0f}))
        game->clearBets();
    ctrlY += 22.0f;

    // Individual bets list (up to 4 lines)
    const char* btNames[] = {"Red","Black","Even","Odd","Doz1","Doz2","Doz3","Straight","1-18","19-36"};
    int shown = 0;
    for (const auto& b : game->getBets()) {
        if (shown++ >= 4) {
            ImGui::SetCursorPos({tableX + 4, ctrlY});
            ImGui::TextDisabled("...");
            ctrlY += 15.0f;
            break;
        }
        ImGui::SetCursorPos({tableX + 4, ctrlY});
        if (b.type == BetType::Straight)
            ImGui::Text("#%d  $%.0f", b.straightNumber, b.amount);
        else
            ImGui::Text("%s  $%.0f", btNames[static_cast<int>(b.type)], b.amount);
        ctrlY += 15.0f;
    }
    if (!idle) ImGui::EndDisabled();

    ctrlY += 8.0f;

    // SPIN button
    bool canSpin = idle && !game->getBets().empty()
                && app.getPlayer() && app.getPlayer()->canBet(game->getTotalBetAmount());
    if (!canSpin) ImGui::BeginDisabled();
    Theme::PushButtonGold();
    ImGui::SetCursorPos({tableX, ctrlY});
    if (ImGui::Button(spinAnim.active ? "  Spinning...  " : "   SPIN!   ", {160.0f, 46.0f})) {
        finalNumber = -1;
        game->play();
        finalNumber = game->getLastNumber();
        int idx2 = findWheelIndex(finalNumber);
        float seg2 = 2.0f * PI_F / WHEEL_COUNT;
        float base2 = -PI_F * 0.5f - idx2 * seg2;
        float rot2  = 8.0f * 2.0f * PI_F;
        float tgt2  = base2 + ceilf((wheelAngle + rot2 - base2) / (2.0f * PI_F)) * 2.0f * PI_F;
        spinStartAngle    = wheelAngle;
        totalSpinAmt      = tgt2 - wheelAngle;
        spinAnim.duration = 6.0f;
        spinAnim.start();
        spinSoundPlayed   = false;
        resultSoundPlayed = false;
    }
    Theme::PopButtonGold();
    if (!canSpin) ImGui::EndDisabled();
    ctrlY += 56.0f;

    // Back to Lobby (always enabled)
    ImGui::SetCursorPos({tableX, ctrlY});
    if (ImGui::Button("  Back to Lobby  ", {160.0f, 36.0f}))
        sm.changeState(std::make_unique<LobbyState>(sm, app));
}

// ---- Main render --------------------------------------------

void RouletteState::render()
{
    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos({0, 0});
    ImGui::SetNextWindowSize(io.DisplaySize);
    ImGui::Begin("##roulette", nullptr,
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoScrollbar  | ImGuiWindowFlags_NoSavedSettings);

    particles.render(ImGui::GetWindowDrawList(), ImGui::GetWindowPos());

    float cx  = io.DisplaySize.x * 0.5f;
    float top = 30.0f;

    // Title
    Theme::PushTitleStyle();
    ImGui::SetWindowFontScale(1.8f);
    ImGui::SetCursorPos({cx - 90.0f, top});
    ImGui::Text("ROULETTE");
    ImGui::SetWindowFontScale(1.0f);
    Theme::PopTitleStyle();

    // Balance
    if (Player* p = app.getPlayer()) {
        ImGui::SetCursorPos({cx - 340.0f, top + 55.0f});
        ImGui::PushStyleColor(ImGuiCol_Text, Theme::Gold());
        ImGui::Text("Balance: $%.2f", p->getBalance());
        ImGui::PopStyleColor();
    }

    // Spinning indicator text
    if (spinAnim.active) {
        int d = (int)(spinAnim.timer * 5) % 4;
        const char* dots[] = {"Spinning.","Spinning..","Spinning...","Spinning   "};
        ImGui::SetCursorPos({cx - 60.0f, top + 55.0f});
        ImGui::PushStyleColor(ImGuiCol_Text, Theme::Gold());
        ImGui::Text("%s", dots[d]);
        ImGui::PopStyleColor();
    }

    // Draw spinning wheel (left/center area)
    float wheelRadius = std::min(cx - 60.0f, 170.0f);
    ImVec2 winPos = ImGui::GetWindowPos();
    float wheelCX = 220.0f;
    float wheelCY = io.DisplaySize.y * 0.5f;
    renderWheel(winPos.x + wheelCX, winPos.y + wheelCY, wheelRadius);

    // Reserve space for the wheel so ImGui layout doesn't overlap
    ImGui::SetCursorPos({0, 0});
    ImGui::Dummy({wheelCX + wheelRadius + 20.0f, io.DisplaySize.y});

    // Result display (below wheel)
    if (showResult && !spinAnim.active) {
        float resY = wheelCY + wheelRadius + 20.0f;
        ImGui::SetCursorPos({wheelCX - wheelRadius, resY});
        bool won = (game->getResult() == GameResult::Win);
        ImGui::PushStyleColor(ImGuiCol_Text, won ? Theme::Gold() : Theme::Red());
        ImGui::SetWindowFontScale(1.3f);
        ImGui::Text("%s", resultMsg.c_str());
        ImGui::SetWindowFontScale(1.0f);
        ImGui::PopStyleColor();

        ImVec2 btnSz{160.0f, 36.0f};
        Theme::PushButtonGold();
        ImGui::SetCursorPos({wheelCX - wheelRadius, resY + 40.0f});
        if (ImGui::Button("  Play Again  ", btnSz)) {
            game = std::make_unique<Roulette>(*app.getPlayer());
            resultMsg.clear();
            showResult        = false;
            spinAnim          = {};
            finalNumber       = -1;
            spinSoundPlayed   = false;
            resultSoundPlayed = false;
            particles.clear();
            AudioManager::instance().play(Sound::ButtonClick, 60);
        }
        Theme::PopButtonGold();
    }

    // Betting table always visible on the right
    float tableX = wheelCX + wheelRadius + 30.0f;
    float tableY = top + 85.0f;
    renderBettingTable(tableX, tableY);

    // Result message handling
    if (spinAnim.finished() && showResult && resultMsg.empty()) {
        int  n   = game->getLastNumber();
        bool red = game->isRedNumber(n);
        bool won = game->getResult() == GameResult::Win;

        if (won)
            resultMsg = std::to_string(n) + (n == 0 ? " GREEN" : red ? " RED" : " BLACK")
                      + "  |  WIN! Payout: $" + std::to_string((int)game->getPayout());
        else if (game->getResult() == GameResult::Push)
            resultMsg = std::to_string(n) + (n == 0 ? " GREEN" : red ? " RED" : " BLACK")
                      + "  |  Push.";
        else
            resultMsg = std::to_string(n) + (n == 0 ? " GREEN" : red ? " RED" : " BLACK")
                      + "  |  Lost.";

        // Achievements
        if (won) {
            AchievementManager::instance().unlock(Achievement::FirstWin);
            // Check if any bet was straight and won
            for (const auto& b : game->getBets())
                if (b.type == BetType::Straight)
                    AchievementManager::instance().unlock(Achievement::LuckyNumber);
        }
        if (game->getTotalBetAmount() >= 1000.0)
            AchievementManager::instance().unlock(Achievement::HighRoller);
        if (app.getPlayer() && app.getPlayer()->getBalance() >= 10000.0)
            AchievementManager::instance().unlock(Achievement::Millionaire);
        if (app.getPlayer() && app.getPlayer()->getBalance() <= 0.0)
            AchievementManager::instance().unlock(Achievement::Broke);
    }

    ImGui::End();
}
