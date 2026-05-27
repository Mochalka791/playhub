#include "BlackjackState.h"
#include "LobbyState.h"
#include "../core/StateManager.h"
#include "../core/Application.h"
#include "../core/AppSettings.h"
#include "../models/Player.h"
#include "../models/AchievementManager.h"
#include "../games/DealerHitSoft17.h"
#include "../games/DealerStandSoft17.h"
#include "../ui/Theme.h"
#include "../audio/AudioManager.h"
#include <imgui.h>
#include <string>

static constexpr float CW = 58.0f;  // card width
static constexpr float CH = 82.0f;  // card height

BlackjackState::BlackjackState(StateManager& sm, Application& app)
    : IState(sm, app) {}

static std::unique_ptr<IBlackjackStrategy> makeStrategy()
{
    int s = AppSettings::instance().dealerStrategy;
    if (s == 1) return std::make_unique<DealerStandSoft17>();
    return std::make_unique<DealerHitSoft17>();
}

void BlackjackState::onEnter()
{
    game = std::make_unique<Blackjack>(*app.getPlayer(), makeStrategy());
    auto& s = AppSettings::instance();
    betAmount  = s.minBet;
    resultMsg.clear();
    dealerTimer = 0.0f;
    resultResolved = false;
    particles.clear();
}

void BlackjackState::update(float dt)
{
    particles.update(dt);

    if (game->getPhase() == BlackjackPhase::DealerTurn) {
        dealerTimer += dt;
        if (dealerTimer >= DEALER_STEP_DELAY)
            dealerTimer = 0.0f;
    }

    if (game->getPhase() == BlackjackPhase::GameOver && !resultResolved) {
        resultResolved = true;
        resolveAndShowResult();
    }
}

void BlackjackState::resolveAndShowResult()
{
    GameResult r = game->getResult();
    double     p = game->getPayout();
    ImGuiIO& io  = ImGui::GetIO();
    float cx = io.DisplaySize.x * 0.5f;
    float cy = io.DisplaySize.y * 0.5f;

    switch (r) {
    case GameResult::Blackjack:
        resultMsg = "BLACKJACK!  Payout: $" + std::to_string((int)p);
        AudioManager::instance().play(Sound::BigWinFanfare, 115);
        particles.emit(cx, cy - 50.0f, 80, ParticleType::Confetti);
        particles.emit(cx, cy - 50.0f, 40, ParticleType::Star);
        particles.emit(cx, cy - 50.0f, 30, ParticleType::Coin);
        AchievementManager::instance().unlock(Achievement::Blackjack);
        AchievementManager::instance().unlock(Achievement::FirstWin);
        break;
    case GameResult::Win:
        resultMsg = "YOU WIN!  Payout: $" + std::to_string((int)p);
        if (game->hasSplit())
            resultMsg += "  +  Split: " +
                (game->getSplitResult() == GameResult::Win
                    ? "$" + std::to_string((int)game->getSplitPayout())
                    : (game->getSplitResult() == GameResult::Push ? "Push" : "Lost"));
        AudioManager::instance().play(Sound::WinFanfare, 115);
        particles.emit(cx, cy - 50.0f, 50, ParticleType::Coin);
        particles.emit(cx, cy - 50.0f, 20, ParticleType::Confetti);
        AchievementManager::instance().unlock(Achievement::FirstWin);
        if (game->hasSplit() && game->getSplitResult() == GameResult::Win)
            AchievementManager::instance().unlock(Achievement::SplitWin);
        break;
    case GameResult::Push:
        resultMsg = "PUSH – Bet returned: $" + std::to_string((int)p);
        AudioManager::instance().play(Sound::CoinDrop, 80);
        break;
    case GameResult::Loss:
        resultMsg = "Dealer wins. You lost $" + std::to_string((int)game->getBet());
        AudioManager::instance().play(Sound::Lose, 90);
        break;
    }

    if (app.getPlayer()) {
        if (app.getPlayer()->getBalance() >= 10000.0)
            AchievementManager::instance().unlock(Achievement::Millionaire);
        if (app.getPlayer()->getBalance() <= 0.0)
            AchievementManager::instance().unlock(Achievement::Broke);
        if (game->getBet() >= 1000.0)
            AchievementManager::instance().unlock(Achievement::HighRoller);
    }
}

// ---- Graphical card drawing ----------------------------------------

void BlackjackState::drawCard(ImDrawList* draw, ImVec2 tl, const Card& card, bool faceDown)
{
    ImU32 bg  = faceDown ? IM_COL32(30, 80, 180, 255) : IM_COL32(252, 248, 236, 255);
    ImU32 brd = IM_COL32(80, 60, 20, 220);
    draw->AddRectFilled(tl, {tl.x + CW, tl.y + CH}, bg, 5.0f);
    draw->AddRect(tl, {tl.x + CW, tl.y + CH}, brd, 5.0f, 0, 1.5f);

    if (faceDown) {
        // Simple pattern on back
        for (int i = 1; i < 5; ++i)
            draw->AddLine({tl.x + i * CW / 5.0f, tl.y + 4},
                          {tl.x + i * CW / 5.0f, tl.y + CH - 4},
                          IM_COL32(50, 110, 220, 100), 1.0f);
        return;
    }

    bool red = (card.suit == Suit::Hearts || card.suit == Suit::Diamonds);
    ImU32 tc = red ? IM_COL32(200, 20, 20, 255) : IM_COL32(10, 10, 10, 255);

    std::string rs = card.rankStr();
    std::string ss = card.suitSymbol();

    draw->AddText({tl.x + 4.0f, tl.y + 3.0f},  tc, rs.c_str());
    draw->AddText({tl.x + 4.0f, tl.y + 16.0f}, tc, ss.c_str());
    // Center suit
    draw->AddText({tl.x + CW * 0.5f - 5.0f, tl.y + CH * 0.5f - 8.0f}, tc, ss.c_str());
    // Bottom-right (mirrored)
    draw->AddText({tl.x + CW - 14.0f, tl.y + CH - 18.0f}, tc, rs.c_str());
    draw->AddText({tl.x + CW - 14.0f, tl.y + CH - 31.0f}, tc, ss.c_str());
}

void BlackjackState::renderHand(ImDrawList* draw, ImVec2 winPos, const Hand& hand,
                                float startX, float y, bool hideFirst) const
{
    for (size_t i = 0; i < hand.size(); ++i) {
        ImVec2 tl = {winPos.x + startX + i * (CW + 6.0f), winPos.y + y};
        drawCard(draw, tl, hand[i], i == 0 && hideFirst);
    }
}

// ---- Render --------------------------------------------------------

void BlackjackState::render()
{
    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos({0, 0});
    ImGui::SetNextWindowSize(io.DisplaySize);
    ImGui::Begin("##bj", nullptr,
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoScrollbar  | ImGuiWindowFlags_NoSavedSettings);

    particles.render(ImGui::GetWindowDrawList(), ImGui::GetWindowPos());
    ImDrawList* draw = ImGui::GetWindowDrawList();
    ImVec2 winPos = ImGui::GetWindowPos();

    float cx  = io.DisplaySize.x * 0.5f;
    float top = 30.0f;

    // Title
    Theme::PushTitleStyle();
    ImGui::SetWindowFontScale(1.8f);
    ImGui::SetCursorPos({cx - 100.0f, top});
    ImGui::Text("BLACKJACK");
    ImGui::SetWindowFontScale(1.0f);
    Theme::PopTitleStyle();

    if (Player* p = app.getPlayer()) {
        ImGui::SetCursorPos({cx - 260.0f, top + 55.0f});
        ImGui::PushStyleColor(ImGuiCol_Text, Theme::Gold());
        ImGui::Text("Balance: $%.2f   |   Bet: $%.2f   |   %s",
                    p->getBalance(), game->getBet(), game->getStrategyName().c_str());
        ImGui::PopStyleColor();
    }

    bool inGame  = (game->getPhase() != BlackjackPhase::Idle
                &&  game->getPhase() != BlackjackPhase::GameOver
                &&  game->getPhase() != BlackjackPhase::InsurancePending);
    bool gameOver = (game->getPhase() == BlackjackPhase::GameOver);

    // ---- Dealer hand ----
    float dealerHandY  = top + 95.0f;
    float playerHandY  = top + 210.0f;
    float splitHandY   = top + 210.0f;
    float dealerStartX = cx - 200.0f;
    float playerStartX = cx - 200.0f;
    float splitStartX  = cx + 20.0f;

    ImGui::SetCursorPos({dealerStartX, dealerHandY - 20.0f});
    ImGui::PushStyleColor(ImGuiCol_Text, Theme::Red());
    ImGui::Text("Dealer  [%d]", game->getPhase() == BlackjackPhase::PlayerTurn ? 0 : handValue(game->getDealerHand()));
    ImGui::PopStyleColor();
    renderHand(draw, winPos, game->getDealerHand(), dealerStartX, dealerHandY,
               game->getPhase() == BlackjackPhase::PlayerTurn);

    ImGui::SetCursorPos({playerStartX, playerHandY - 20.0f});
    ImGui::PushStyleColor(ImGuiCol_Text, Theme::Gold());
    ImGui::Text("You  [%d]", handValue(game->getPlayerHand()));
    ImGui::PopStyleColor();
    renderHand(draw, winPos, game->getPlayerHand(), playerStartX, playerHandY, false);

    // Split hand
    if (game->hasSplit()) {
        ImGui::SetCursorPos({splitStartX, splitHandY - 20.0f});
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{0.4f, 0.8f, 1.0f, 1.0f});
        ImGui::Text("Split  [%d]", handValue(game->getSplitHand()));
        ImGui::PopStyleColor();
        renderHand(draw, winPos, game->getSplitHand(), splitStartX, splitHandY, false);

        // Active hand indicator
        if (game->getPhase() == BlackjackPhase::SplitTurn) {
            ImGui::SetCursorPos({splitStartX, splitHandY + CH + 4.0f});
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{0.4f, 0.8f, 1.0f, 1.0f});
            ImGui::Text("^ Active");
            ImGui::PopStyleColor();
        } else if (game->getPhase() == BlackjackPhase::PlayerTurn) {
            ImGui::SetCursorPos({playerStartX, playerHandY + CH + 4.0f});
            ImGui::PushStyleColor(ImGuiCol_Text, Theme::Gold());
            ImGui::Text("^ Active");
            ImGui::PopStyleColor();
        }
    }

    float actionY = top + 330.0f;

    // ---- Insurance pending ----
    if (game->getPhase() == BlackjackPhase::InsurancePending) {
        ImGui::SetCursorPos({cx - 200.0f, actionY});
        ImGui::PushStyleColor(ImGuiCol_Text, Theme::Gold());
        ImGui::Text("Dealer shows Ace! Take insurance? (Costs %.0f$, pays 2:1 if dealer has BJ)",
                    game->getBet() * 0.5);
        ImGui::PopStyleColor();
        actionY += 35.0f;
        ImVec2 btnSz{160.0f, 40.0f};
        Theme::PushButtonGold();
        ImGui::SetCursorPos({cx - 200.0f, actionY});
        if (ImGui::Button("  Take Insurance  ", btnSz)) {
            game->takeInsurance();
            AudioManager::instance().play(Sound::Chip, 80);
        }
        Theme::PopButtonGold();
        ImGui::SetCursorPos({cx - 30.0f, actionY});
        if (ImGui::Button("  Decline  ", btnSz)) {
            game->declineInsurance();
            AudioManager::instance().play(Sound::ButtonClick, 60);
        }
    }

    // ---- Idle: bet + deal ----
    if (game->getPhase() == BlackjackPhase::Idle) {
        auto& s = AppSettings::instance();
        ImGui::SetCursorPos({cx - 200.0f, actionY});
        ImGui::Text("Bet Amount:");
        ImGui::SetCursorPos({cx - 200.0f, actionY + 25.0f});
        ImGui::SetNextItemWidth(300.0f);
        float maxBet = app.getPlayer()
            ? std::min((float)app.getPlayer()->getBalance(), s.maxBet) : 100.0f;
        ImGui::SliderFloat("##bjbet", &betAmount, s.minBet, std::max(s.minBet, maxBet), "%.0f $");

        bool canDeal = app.getPlayer() && app.getPlayer()->canBet(betAmount);
        ImGui::SetCursorPos({cx - 80.0f, actionY + 80.0f});
        Theme::PushButtonGold();
        if (!canDeal) ImGui::BeginDisabled();
        if (ImGui::Button("  DEAL  ", {160.0f, 45.0f})) {
            game->placeBet(betAmount);
            game->play();
            resultMsg.clear();
            resultResolved = false;
            AudioManager::instance().play(Sound::CardFlip, 80);
        }
        if (!canDeal) ImGui::EndDisabled();
        Theme::PopButtonGold();

        ImGui::SetCursorPos({cx - 80.0f, actionY + 135.0f});
        if (ImGui::Button("  Back to Lobby  ", {160.0f, 38.0f}))
            sm.changeState(std::make_unique<LobbyState>(sm, app));
    }

    // ---- Player turn action buttons ----
    if (game->getPhase() == BlackjackPhase::PlayerTurn
     || game->getPhase() == BlackjackPhase::SplitTurn) {
        ImVec2 btnSize{120.0f, 40.0f};
        float bx = cx - 260.0f;

        Theme::PushButtonGold();
        ImGui::SetCursorPos({bx, actionY});
        if (ImGui::Button("  HIT  ", btnSize)) {
            game->hit();
            AudioManager::instance().play(Sound::CardFlip, 70);
        }
        bx += 130.0f;
        ImGui::SetCursorPos({bx, actionY});
        if (ImGui::Button("  STAND  ", btnSize)) {
            game->stand();
            AudioManager::instance().play(Sound::ButtonClick, 70);
        }
        bx += 130.0f;
        Theme::PopButtonGold();

        if (game->canDoubleDown()) {
            ImGui::SetCursorPos({bx, actionY});
            if (ImGui::Button("  DOUBLE  ", btnSize)) {
                game->doubleDown();
                AudioManager::instance().play(Sound::CardFlip, 80);
            }
            bx += 130.0f;
        }

        if (game->canSplit()) {
            ImGui::SetCursorPos({bx, actionY});
            if (ImGui::Button("  SPLIT  ", btnSize)) {
                game->split();
                AudioManager::instance().play(Sound::CardFlip, 80);
            }
        }

        if (isBust(game->getPhase() == BlackjackPhase::SplitTurn
                   ? game->getSplitHand() : game->getPlayerHand())) {
            ImGui::SetCursorPos({cx - 30.0f, actionY + 48.0f});
            ImGui::PushStyleColor(ImGuiCol_Text, Theme::Red());
            ImGui::Text("BUST!");
            ImGui::PopStyleColor();
        }
    }

    // ---- Dealer turn indicator ----
    if (game->getPhase() == BlackjackPhase::DealerTurn) {
        ImGui::SetCursorPos({cx - 100.0f, actionY});
        ImGui::PushStyleColor(ImGuiCol_Text, Theme::Gold());
        ImGui::Text("Dealer is playing...");
        ImGui::PopStyleColor();
    }

    // ---- Result ----
    if (gameOver && !resultMsg.empty()) {
        ImGui::SetCursorPos({cx - 260.0f, actionY});
        bool won = (game->getResult() == GameResult::Win
                 || game->getResult() == GameResult::Blackjack);
        ImGui::PushStyleColor(ImGuiCol_Text, won ? Theme::Gold() : Theme::Red());
        ImGui::SetWindowFontScale(1.3f);
        ImGui::Text("%s", resultMsg.c_str());
        ImGui::SetWindowFontScale(1.0f);
        ImGui::PopStyleColor();

        // Insurance result line
        if (game->getInsuranceBet() > 0.0) {
            ImGui::SetCursorPos({cx - 260.0f, actionY + 30.0f});
            ImGui::PushStyleColor(ImGuiCol_Text, game->insuranceWon() ? Theme::Gold() : Theme::Red());
            ImGui::Text("Insurance: %s", game->insuranceWon() ? "WON +$" + std::to_string((int)(game->getInsuranceBet() * 2)) + " " : "Lost");
            ImGui::PopStyleColor();
        }

        ImVec2 btnSize{170.0f, 40.0f};
        Theme::PushButtonGold();
        ImGui::SetCursorPos({cx - 180.0f, actionY + 65.0f});
        if (ImGui::Button("  Play Again  ", btnSize)) {
            game = std::make_unique<Blackjack>(*app.getPlayer(), makeStrategy());
            resultMsg.clear();
            dealerTimer = 0.0f;
            resultResolved = false;
            particles.clear();
            AudioManager::instance().play(Sound::ButtonClick, 60);
        }
        Theme::PopButtonGold();

        ImGui::SetCursorPos({cx + 10.0f, actionY + 65.0f});
        if (ImGui::Button("  Back to Lobby  ", btnSize))
            sm.changeState(std::make_unique<LobbyState>(sm, app));
    }

    ImGui::End();
}
