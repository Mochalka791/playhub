#include "VideoPokerState.h"
#include "LobbyState.h"
#include "../core/StateManager.h"
#include "../core/Application.h"
#include "../core/AppSettings.h"
#include "../models/Player.h"
#include "../models/AchievementManager.h"
#include "../ui/Theme.h"
#include "../audio/AudioManager.h"
#include <imgui.h>
#include <string>

static constexpr float CARD_W = 72.0f;
static constexpr float CARD_H = 100.0f;

VideoPokerState::VideoPokerState(StateManager& sm, Application& app)
    : IState(sm, app) {}

void VideoPokerState::onEnter()
{
    game = std::make_unique<VideoPoker>(*app.getPlayer());
    auto& s = AppSettings::instance();
    betAmount = s.minBet;
    resultMsg.clear();
    resultResolved = false;
    particles.clear();
}

void VideoPokerState::update(float dt)
{
    particles.update(dt);

    if (game->getPhase() == VideoPoker::Phase::GameOver && !resultResolved) {
        resultResolved = true;
        resolveAchievements();

        GameResult r  = game->getResult();
        PokerRank  hr = game->getHandRank();

        if (r == GameResult::Win) {
            resultMsg = VideoPoker::rankName(hr) + "!  Payout: $"
                      + std::to_string((int)game->getPayout());
            ImGuiIO& io = ImGui::GetIO();
            float cx = io.DisplaySize.x * 0.5f;
            float cy = io.DisplaySize.y * 0.5f;
            if (hr >= PokerRank::FourOfAKind) {
                AudioManager::instance().play(Sound::BigWinFanfare, 115);
                particles.emit(cx, cy, 100, ParticleType::Confetti);
                particles.emit(cx, cy, 50, ParticleType::Star);
                particles.emit(cx, cy, 40, ParticleType::Coin);
            } else {
                AudioManager::instance().play(Sound::WinFanfare, 110);
                particles.emit(cx, cy, 50, ParticleType::Coin);
                particles.emit(cx, cy, 20, ParticleType::Confetti);
            }
        } else {
            resultMsg = "No win. " + VideoPoker::rankName(hr);
            AudioManager::instance().play(Sound::Lose, 90);
        }
    }
}

void VideoPokerState::resolveAchievements()
{
    auto& am = AchievementManager::instance();
    if (game->getResult() == GameResult::Win)
        am.unlock(Achievement::FirstWin);
    if (game->getHandRank() == PokerRank::RoyalFlush)
        am.unlock(Achievement::RoyalFlush);
    if (game->getBet() >= 1000.0)
        am.unlock(Achievement::HighRoller);
    if (app.getPlayer() && app.getPlayer()->getBalance() >= 10000.0)
        am.unlock(Achievement::Millionaire);
    if (app.getPlayer() && app.getPlayer()->getBalance() <= 0.0)
        am.unlock(Achievement::Broke);
}

// ---- Card rendering -----------------------------------------

void VideoPokerState::drawCard(ImDrawList* draw, ImVec2 tl, const Card& card, bool held) const
{
    // Background
    ImU32 bg  = IM_COL32(250, 248, 240, 255);
    ImU32 brd = held ? IM_COL32(50, 200, 50, 255) : IM_COL32(120, 100, 60, 200);
    float bw  = held ? 3.0f : 1.5f;

    draw->AddRectFilled(tl, {tl.x + CARD_W, tl.y + CARD_H}, bg, 6.0f);
    draw->AddRect(tl, {tl.x + CARD_W, tl.y + CARD_H}, brd, 6.0f, 0, bw);

    bool red = (card.suit == Suit::Hearts || card.suit == Suit::Diamonds);
    ImU32 textCol = red ? IM_COL32(200, 20, 20, 255) : IM_COL32(10, 10, 10, 255);

    // Rank top-left
    std::string rs = card.rankStr();
    draw->AddText({tl.x + 5.0f, tl.y + 4.0f}, textCol, rs.c_str());

    // Suit center
    std::string ss = card.suitSymbol();
    draw->AddText({tl.x + CARD_W * 0.5f - 6.0f, tl.y + CARD_H * 0.5f - 8.0f}, textCol, ss.c_str());

    // Rank bottom-right (flipped)
    draw->AddText({tl.x + CARD_W - 14.0f, tl.y + CARD_H - 18.0f}, textCol, rs.c_str());

    // HELD label
    if (held) {
        draw->AddRectFilled({tl.x + 2.0f, tl.y + CARD_H + 4.0f},
                            {tl.x + CARD_W - 2.0f, tl.y + CARD_H + 20.0f},
                            IM_COL32(30, 180, 30, 220), 3.0f);
        draw->AddText({tl.x + 16.0f, tl.y + CARD_H + 5.0f}, IM_COL32(255, 255, 255, 255), "HELD");
    }
}

void VideoPokerState::render()
{
    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos({0, 0});
    ImGui::SetNextWindowSize(io.DisplaySize);
    ImGui::Begin("##vp", nullptr,
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoScrollbar  | ImGuiWindowFlags_NoSavedSettings);

    particles.render(ImGui::GetWindowDrawList(), ImGui::GetWindowPos());

    float cx  = io.DisplaySize.x * 0.5f;
    float top = 30.0f;

    Theme::PushTitleStyle();
    ImGui::SetWindowFontScale(1.8f);
    ImGui::SetCursorPos({cx - 110.0f, top});
    ImGui::Text("VIDEO POKER");
    ImGui::SetWindowFontScale(1.0f);
    Theme::PopTitleStyle();

    if (Player* p = app.getPlayer()) {
        ImGui::SetCursorPos({cx - 200.0f, top + 55.0f});
        ImGui::PushStyleColor(ImGuiCol_Text, Theme::Gold());
        ImGui::Text("Balance: $%.2f", p->getBalance());
        ImGui::PopStyleColor();
    }

    // Paytable
    ImGui::SetCursorPos({cx + 80.0f, top + 90.0f});
    ImGui::PushStyleColor(ImGuiCol_Text, Theme::DarkGold());
    ImGui::Text("Paytable:");
    const char* ptLines[] = {
        "Royal Flush   800x", "Straight Flush  50x",
        "Four of a Kind 25x", "Full House       9x",
        "Flush           6x", "Straight         4x",
        "Three of a Kind  3x","Two Pair         2x",
        "Jacks or Better 1x"
    };
    for (int i = 0; i < 9; ++i) {
        ImGui::SetCursorPos({cx + 80.0f, top + 112.0f + i * 18.0f});
        ImGui::Text("%s", ptLines[i]);
    }
    ImGui::PopStyleColor();

    // --- Cards ---
    float cardRowY = top + 110.0f;
    float totalW   = 5 * CARD_W + 4 * 16.0f;
    float startX   = cx - totalW * 0.5f;
    ImVec2 winPos  = ImGui::GetWindowPos();
    ImDrawList* draw = ImGui::GetWindowDrawList();

    if (game->getPhase() != VideoPoker::Phase::Idle) {
        const auto& h = game->getHand();
        const auto& ho = game->getHolds();
        for (int i = 0; i < 5; ++i) {
            ImVec2 tl = {winPos.x + startX + i * (CARD_W + 16.0f), winPos.y + cardRowY};
            drawCard(draw, tl, h[i], ho[i]);
        }
    }

    float btnY = cardRowY + CARD_H + 40.0f;

    // --- Idle: bet + deal ---
    if (game->getPhase() == VideoPoker::Phase::Idle) {
        auto& s = AppSettings::instance();
        ImGui::SetCursorPos({cx - 200.0f, btnY - 50.0f});
        ImGui::Text("Bet Amount:");
        ImGui::SetCursorPos({cx - 200.0f, btnY - 25.0f});
        ImGui::SetNextItemWidth(280.0f);
        float maxBet = app.getPlayer() ? std::min((float)app.getPlayer()->getBalance(), s.maxBet) : 100.0f;
        ImGui::SliderFloat("##vpbet", &betAmount, s.minBet, std::max(s.minBet, maxBet), "%.0f $");

        bool canDeal = app.getPlayer() && app.getPlayer()->canBet(betAmount);
        ImGui::SetCursorPos({cx - 80.0f, btnY + 10.0f});
        Theme::PushButtonGold();
        if (!canDeal) ImGui::BeginDisabled();
        if (ImGui::Button("  DEAL  ", {160.0f, 46.0f})) {
            game->placeBet(betAmount);
            game->play();
            resultMsg.clear();
            resultResolved = false;
            AudioManager::instance().play(Sound::CardFlip, 80);
        }
        if (!canDeal) ImGui::EndDisabled();
        Theme::PopButtonGold();

        ImGui::SetCursorPos({cx - 80.0f, btnY + 66.0f});
        if (ImGui::Button("  Back to Lobby  ", {160.0f, 36.0f}))
            sm.changeState(std::make_unique<LobbyState>(sm, app));
    }

    // --- Holding: click cards to hold + draw button ---
    if (game->getPhase() == VideoPoker::Phase::Holding) {
        ImGui::SetCursorPos({cx - 200.0f, cardRowY - 24.0f});
        ImGui::PushStyleColor(ImGuiCol_Text, Theme::Gold());
        ImGui::Text("Click a card to hold it, then DRAW.");
        ImGui::PopStyleColor();

        // Invisible buttons over each card for hold toggle
        for (int i = 0; i < 5; ++i) {
            float cardX = startX + i * (CARD_W + 16.0f);
            ImGui::SetCursorPos({cardX, cardRowY});
            std::string id = "##hld" + std::to_string(i);
            if (ImGui::InvisibleButton(id.c_str(), {CARD_W, CARD_H})) {
                bool cur = game->getHolds()[i];
                game->setHold(i, !cur);
                AudioManager::instance().play(Sound::CardFlip, 50);
            }
        }

        ImGui::SetCursorPos({cx - 80.0f, btnY + 10.0f});
        Theme::PushButtonGold();
        if (ImGui::Button("  DRAW  ", {160.0f, 46.0f})) {
            game->draw();
            AudioManager::instance().play(Sound::CardFlip, 70);
        }
        Theme::PopButtonGold();
    }

    // --- GameOver: result + play again ---
    if (game->getPhase() == VideoPoker::Phase::GameOver) {
        ImGui::SetCursorPos({cx - 200.0f, btnY - 10.0f});
        bool won = (game->getResult() == GameResult::Win);
        ImGui::PushStyleColor(ImGuiCol_Text, won ? Theme::Gold() : Theme::Red());
        ImGui::SetWindowFontScale(1.3f);
        ImGui::Text("%s", resultMsg.c_str());
        ImGui::SetWindowFontScale(1.0f);
        ImGui::PopStyleColor();

        ImVec2 btnSz{170.0f, 40.0f};
        Theme::PushButtonGold();
        ImGui::SetCursorPos({cx - 180.0f, btnY + 40.0f});
        if (ImGui::Button("  Play Again  ", btnSz)) {
            game = std::make_unique<VideoPoker>(*app.getPlayer());
            resultMsg.clear();
            resultResolved = false;
            particles.clear();
            AudioManager::instance().play(Sound::ButtonClick, 60);
        }
        Theme::PopButtonGold();

        ImGui::SetCursorPos({cx + 10.0f, btnY + 40.0f});
        if (ImGui::Button("  Back to Lobby  ", btnSz))
            sm.changeState(std::make_unique<LobbyState>(sm, app));
    }

    ImGui::End();
}
