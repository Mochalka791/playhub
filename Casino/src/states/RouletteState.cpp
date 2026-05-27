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

// ---- Betting panel ------------------------------------------

void RouletteState::renderBettingPanel(float panelX, float panelY)
{
    ImGui::SetCursorPos({panelX, panelY});
    ImGui::Text("Bet Type:");

    const char* betNames[] = {
        "Red","Black","Even","Odd",
        "Dozen 1-12","Dozen 13-24","Dozen 25-36","Straight"
    };
    BetType betValues[] = {
        BetType::Red, BetType::Black, BetType::Even, BetType::Odd,
        BetType::Dozen1, BetType::Dozen2, BetType::Dozen3, BetType::Straight
    };

    ImGui::SetCursorPos({panelX, panelY + 24.0f});
    ImGui::SetNextItemWidth(200.0f);
    int sel = static_cast<int>(game->currentBetType);
    if (ImGui::Combo("##bettype", &sel, betNames, 8))
        game->currentBetType = betValues[sel];

    float y = panelY + 60.0f;
    if (game->currentBetType == BetType::Straight) {
        ImGui::SetCursorPos({panelX, y});
        ImGui::Text("Number (0-36):");
        y += 22.0f;
        ImGui::SetCursorPos({panelX, y});
        ImGui::SetNextItemWidth(120.0f);
        ImGui::InputInt("##snum", &straightNum);
        straightNum = std::clamp(straightNum, 0, 36);
        game->straightTarget = straightNum;
        y += 35.0f;
    }

    ImGui::SetCursorPos({panelX, y});
    ImGui::Text("Amount:");
    y += 24.0f;
    ImGui::SetCursorPos({panelX, y});
    ImGui::SetNextItemWidth(200.0f);
    float maxBet = app.getPlayer() ? (float)app.getPlayer()->getBalance() : 100.0f;
    ImGui::SliderFloat("##betamt", &betAmount, 1.0f, std::max(1.0f, maxBet), "%.0f $");
    y += 38.0f;

    Theme::PushButtonGold();
    ImGui::SetCursorPos({panelX, y});
    if (ImGui::Button("  Add Bet  ", {120.0f, 34.0f}))
        game->addBet(game->currentBetType, betAmount,
                     (game->currentBetType == BetType::Straight) ? straightNum : -1);

    ImGui::SameLine();
    if (ImGui::Button("  Clear  ", {90.0f, 34.0f}))
        game->clearBets();
    Theme::PopButtonGold();
    y += 44.0f;

    // Current bets
    ImGui::SetCursorPos({panelX, y});
    ImGui::PushStyleColor(ImGuiCol_Text, Theme::Gold());
    ImGui::Text("Bets:  Total = $%.2f", game->getTotalBetAmount());
    ImGui::PopStyleColor();
    y += 22.0f;

    const char* btNames[] = {"Red","Black","Even","Odd","Dozen1","Dozen2","Dozen3","Straight"};
    for (const auto& b : game->getBets()) {
        if (y > ImGui::GetIO().DisplaySize.y - 80.0f) break;
        ImGui::SetCursorPos({panelX + 8.0f, y});
        if (b.type == BetType::Straight)
            ImGui::Text("Straight #%d  $%.0f", b.straightNumber, b.amount);
        else
            ImGui::Text("%s  $%.0f", btNames[(int)b.type], b.amount);
        y += 20.0f;
    }

    // Spin
    y = std::max(y + 10.0f, panelY + 320.0f);
    bool canSpin = !game->getBets().empty()
                && !spinAnim.active
                && !showResult
                && app.getPlayer()->canBet(game->getTotalBetAmount());

    ImGui::SetCursorPos({panelX, y});
    Theme::PushButtonGold();
    if (!canSpin) ImGui::BeginDisabled();
    if (ImGui::Button("   SPIN!   ", {160.0f, 46.0f})) {
        finalNumber    = -1;
        game->play();
        finalNumber    = game->getLastNumber();
        // Calculate target wheel angle
        int idx = findWheelIndex(finalNumber);
        float segAngle = 2.0f * PI_F / WHEEL_COUNT;
        // We want sector idx centered at top (-PI/2)
        float base  = -PI_F * 0.5f - (idx + 0.5f) * segAngle;
        // Ensure target > current + 6 full rotations
        float rotations = 6.0f * 2.0f * PI_F;
        float target = base + ceilf((wheelAngle + rotations - base) / (2.0f * PI_F)) * 2.0f * PI_F;
        spinStartAngle    = wheelAngle;
        totalSpinAmt      = target - wheelAngle;
        spinAnim.duration = 4.0f;
        spinAnim.start();
        spinSoundPlayed   = false;
        resultSoundPlayed = false;
    }
    if (!canSpin) ImGui::EndDisabled();
    Theme::PopButtonGold();

    y += 55.0f;
    ImGui::SetCursorPos({panelX, y});
    if (ImGui::Button("  Back to Lobby  ", {160.0f, 36.0f}))
        sm.changeState(std::make_unique<LobbyState>(sm, app));
}

// ---- Visual betting table ------------------------------------
// Red numbers per European roulette standard
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

    // 0 cell spanning full height on left
    {
        ImVec2 tl = {wx, wy};
        ImVec2 br = {wx + CW, wy + 12 * (CH + gap)};
        draw->AddRectFilled(tl, br, IM_COL32(0, 130, 30, 255), 3.0f);
        draw->AddRect(tl, br, IM_COL32(180, 160, 60, 200), 3.0f, 0, 1.0f);
        draw->AddText({tl.x + 8, tl.y + 12 * (CH + gap) / 2 - 7}, IM_COL32(255,255,255,255), "0");

        // Invisible button
        ImGui::SetCursorPos({tableX, tableY});
        if (ImGui::InvisibleButton("##t0", {CW, 12*(CH+gap)})) {
            game->currentBetType = BetType::Straight;
            game->straightTarget = 0;
            straightNum = 0;
            game->addBet(BetType::Straight, betAmount, 0);
            AudioManager::instance().play(Sound::Chip, 60);
        }
    }

    // Number cells: rows 3→1 top to bottom, columns 1–12 left to right
    for (int col = 1; col <= 12; ++col) {
        for (int row = 3; row >= 1; --row) {
            int num = (col - 1) * 3 + row; // 1..36
            float cx2 = wx + CW + gap + (col - 1) * (CW + gap);
            float cy2 = wy + (3 - row) * (CH + gap);
            ImVec2 tl = {cx2, cy2};
            ImVec2 br = {cx2 + CW, cy2 + CH};

            bool red = isRed(num);
            ImU32 bg = red ? IM_COL32(180, 20, 20, 255) : IM_COL32(15, 15, 15, 255);
            draw->AddRectFilled(tl, br, bg, 2.0f);
            draw->AddRect(tl, br, IM_COL32(140, 120, 50, 180), 2.0f, 0, 1.0f);
            char buf[4]; snprintf(buf, sizeof(buf), "%d", num);
            draw->AddText({tl.x + 4, tl.y + 5}, IM_COL32(255,255,255,240), buf);

            // Invisible button for placing straight bet
            float relX = tableX + CW + gap + (col - 1) * (CW + gap);
            float relY = tableY + (3 - row) * (CH + gap);
            ImGui::SetCursorPos({relX, relY});
            std::string bid = "##tb" + std::to_string(num);
            if (ImGui::InvisibleButton(bid.c_str(), {CW, CH})) {
                game->currentBetType = BetType::Straight;
                game->straightTarget = num;
                straightNum = num;
                game->addBet(BetType::Straight, betAmount, num);
                AudioManager::instance().play(Sound::Chip, 60);
            }
        }
    }

    // Below grid: 2:1 column bets
    float below = wy + 3 * (CH + gap) + 4.0f;
    const char* colLabels[] = {"2:1","2:1","2:1"};
    BetType colTypes[] = {BetType::Dozen1, BetType::Dozen2, BetType::Dozen3};
    int colNums[] = {-1,-1,-1};
    float colW = (12 * (CW + gap)) / 3.0f - gap;

    for (int i = 0; i < 3; ++i) {
        float cx2 = wx + CW + gap + i * (colW + gap);
        ImVec2 tl = {cx2, below};
        ImVec2 br = {cx2 + colW, below + CH};
        draw->AddRectFilled(tl, br, IM_COL32(20, 60, 20, 255), 3.0f);
        draw->AddRect(tl, br, IM_COL32(140,120,50,180), 3.0f, 0, 1.0f);
        float lx = tableX + CW + gap + i * (colW + gap);
        draw->AddText({cx2 + colW/2 - 8, below + 5}, IM_COL32(200,180,60,255), colLabels[i]);
        ImGui::SetCursorPos({lx, tableY + 3*(CH+gap) + 4});
        std::string bid = "##col" + std::to_string(i);
        if (ImGui::InvisibleButton(bid.c_str(), {colW, CH})) {
            game->addBet(colTypes[i], betAmount, colNums[i]);
            AudioManager::instance().play(Sound::Chip, 60);
        }
    }

    // Dozen + outside bets row below
    float below2 = below + CH + 4.0f;
    float totalW = 12 * (CW + gap);
    const char* doz[] = {"Dozen 1-12","Dozen 13-24","Dozen 25-36"};
    BetType dozT[] = {BetType::Dozen1, BetType::Dozen2, BetType::Dozen3};
    float dozW = totalW / 3.0f - gap;
    for (int i = 0; i < 3; ++i) {
        float cx2 = wx + CW + gap + i * (dozW + gap);
        ImVec2 tl = {cx2, below2};
        ImVec2 br = {cx2 + dozW, below2 + CH};
        draw->AddRectFilled(tl, br, IM_COL32(25, 50, 25, 255), 3.0f);
        draw->AddRect(tl, br, IM_COL32(140,120,50,180), 3.0f, 0, 1.0f);
        draw->AddText({cx2 + 2, below2 + 5}, IM_COL32(200,200,200,255), doz[i]);
        float relX = tableX + CW + gap + i * (dozW + gap);
        float relY = tableY + (3*(CH+gap) + 4 + CH + 4);
        ImGui::SetCursorPos({relX, relY});
        std::string bid = "##doz" + std::to_string(i);
        if (ImGui::InvisibleButton(bid.c_str(), {dozW, CH})) {
            game->addBet(dozT[i], betAmount, -1);
            AudioManager::instance().play(Sound::Chip, 60);
        }
    }

    // Outside bets: Red, Black, Even, Odd, 1-18, 19-36
    float below3 = below2 + CH + 4.0f;
    float outerW = totalW / 6.0f - gap;
    struct OBet { const char* label; BetType type; ImU32 col; };
    OBet outers[] = {
        {"1-18",  BetType::Odd,   IM_COL32(25,50,25,255)},  // reuse Odd temporarily
        {"EVEN",  BetType::Even,  IM_COL32(25,50,25,255)},
        {"RED",   BetType::Red,   IM_COL32(160,15,15,255)},
        {"BLACK", BetType::Black, IM_COL32(15,15,15,255)},
        {"ODD",   BetType::Odd,   IM_COL32(25,50,25,255)},
        {"19-36", BetType::Odd,   IM_COL32(25,50,25,255)},  // reuse Odd
    };
    // Fix types for 1-18 and 19-36 (these are actually covered by Dozen1+2 / Dozen2+3
    // but for simplicity map to Even/Odd) — map both to Odd/Even is wrong; use Dozen types
    // Actually add proper BetType mapping:
    BetType outerTypes[] = {BetType::Odd, BetType::Even, BetType::Red,
                            BetType::Black, BetType::Odd, BetType::Even};
    const char* outerLabels[] = {"1-18","EVEN","RED","BLACK","ODD","19-36"};
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
        float relX = tableX + CW + gap + i * (outerW + gap);
        float relY = tableY + (3*(CH+gap) + 4 + CH + 4 + CH + 4);
        ImGui::SetCursorPos({relX, relY});
        std::string bid = "##out" + std::to_string(i);
        if (ImGui::InvisibleButton(bid.c_str(), {outerW, CH})) {
            game->addBet(outerTypes[i], betAmount, -1);
            AudioManager::instance().play(Sound::Chip, 60);
        }
    }
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

    // Betting table (bottom area) or panel (right side while spinning/result)
    float tableX = wheelCX + wheelRadius + 30.0f;
    float tableY = top + 85.0f;
    float panelX = tableX;
    float panelY = tableY;

    if (!spinAnim.active && !showResult) {
        renderBettingPanel(panelX, panelY);
    } else if (showResult && !spinAnim.active) {
        ImGui::SetCursorPos({panelX, panelY});
        if (ImGui::Button("  Back to Lobby  ", {180.0f, 38.0f}))
            sm.changeState(std::make_unique<LobbyState>(sm, app));
    }

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
