#include "BlackjackState.h"
#include "LobbyState.h"
#include "../core/StateManager.h"
#include "../core/Application.h"
#include "../models/Player.h"
#include "../games/DealerHitSoft17.h"
#include "../ui/Theme.h"
#include <imgui.h>
#include <string>

BlackjackState::BlackjackState(StateManager& sm, Application& app)
    : IState(sm, app) {}

void BlackjackState::onEnter()
{
    game = std::make_unique<Blackjack>(
        *app.getPlayer(),
        std::make_unique<DealerHitSoft17>());
    betAmount  = 10.0f;
    resultMsg.clear();
    dealerTimer = 0.0f;
}

void BlackjackState::update(float dt)
{
    // Dealer-turn auto-play with timed delay between hits
    if (game->getPhase() == BlackjackPhase::DealerTurn) {
        dealerTimer += dt;
        if (dealerTimer >= DEALER_STEP_DELAY) {
            dealerTimer = 0.0f;
            // Dealer plays are already fully resolved inside game->stand()
            // We just wait for GameOver
        }
    }

    if (game->getPhase() == BlackjackPhase::GameOver && resultMsg.empty()) {
        resolveAndShowResult();
    }
}

void BlackjackState::resolveAndShowResult()
{
    GameResult r = game->getResult();
    double     p = game->getPayout();
    switch (r) {
    case GameResult::Blackjack:
        resultMsg = "BLACKJACK!  Payout: $" + std::to_string((int)p);
        break;
    case GameResult::Win:
        resultMsg = "YOU WIN!  Payout: $" + std::to_string((int)p);
        break;
    case GameResult::Push:
        resultMsg = "PUSH – Bet returned: $" + std::to_string((int)p);
        break;
    case GameResult::Loss:
        resultMsg = "Dealer wins. You lost $" + std::to_string((int)game->getBet());
        break;
    }
}

void BlackjackState::renderHand(const Hand& hand, bool hideFirst, const char* label)
{
    ImGui::Text("%s", label);
    ImGui::SameLine();
    for (size_t i = 0; i < hand.size(); ++i) {
        if (i == 0 && hideFirst) {
            ImGui::SameLine();
            ImGui::Text("[??]");
        } else {
            ImGui::SameLine();
            ImGui::Text("[%s]", hand[i].toString().c_str());
        }
    }
    if (!hideFirst || hand.empty()) {
        ImGui::SameLine();
        ImGui::Text("= %d", handValue(hand));
    }
}

void BlackjackState::render()
{
    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos({0, 0});
    ImGui::SetNextWindowSize(io.DisplaySize);
    ImGui::Begin("##bj", nullptr,
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoScrollbar  | ImGuiWindowFlags_NoSavedSettings);

    float cx  = io.DisplaySize.x * 0.5f;
    float top = 30.0f;

    // Title
    Theme::PushTitleStyle();
    ImGui::SetWindowFontScale(1.8f);
    ImGui::SetCursorPos({cx - 100.0f, top});
    ImGui::Text("BLACKJACK");
    ImGui::SetWindowFontScale(1.0f);
    Theme::PopTitleStyle();

    // Balance
    if (Player* p = app.getPlayer()) {
        ImGui::SetCursorPos({cx - 200.0f, top + 55.0f});
        ImGui::PushStyleColor(ImGuiCol_Text, Theme::Gold());
        ImGui::Text("Balance: $%.2f   |   Bet: $%.2f   |   Strategy: %s",
                    p->getBalance(), game->getBet(), game->getStrategyName().c_str());
        ImGui::PopStyleColor();
    }

    bool inGame = (game->getPhase() != BlackjackPhase::Idle
               &&  game->getPhase() != BlackjackPhase::GameOver);
    bool gameOver = (game->getPhase() == BlackjackPhase::GameOver);

    // ---- Dealer hand ----
    ImGui::SetCursorPos({cx - 200.0f, top + 110.0f});
    bool hideDealer = (game->getPhase() == BlackjackPhase::PlayerTurn);
    ImGui::SetWindowFontScale(1.3f);
    ImGui::PushStyleColor(ImGuiCol_Text, Theme::Red());
    renderHand(game->getDealerHand(), hideDealer, "Dealer: ");
    ImGui::PopStyleColor();
    ImGui::SetWindowFontScale(1.0f);

    // ---- Player hand ----
    ImGui::SetCursorPos({cx - 200.0f, top + 160.0f});
    ImGui::SetWindowFontScale(1.3f);
    ImGui::PushStyleColor(ImGuiCol_Text, Theme::Gold());
    renderHand(game->getPlayerHand(), false, "You:    ");
    ImGui::PopStyleColor();
    ImGui::SetWindowFontScale(1.0f);

    // ---- Bet input (only when idle) ----
    if (game->getPhase() == BlackjackPhase::Idle) {
        ImGui::SetCursorPos({cx - 200.0f, top + 225.0f});
        ImGui::Text("Bet Amount:");
        ImGui::SetCursorPos({cx - 200.0f, top + 250.0f});
        ImGui::SetNextItemWidth(300.0f);
        float maxBet = app.getPlayer() ? (float)app.getPlayer()->getBalance() : 100.0f;
        ImGui::SliderFloat("##bjbet", &betAmount, 1.0f, std::max(1.0f, maxBet), "%.0f $");

        bool canDeal = app.getPlayer() && app.getPlayer()->canBet(betAmount);

        ImGui::SetCursorPos({cx - 80.0f, top + 305.0f});
        Theme::PushButtonGold();
        if (!canDeal) ImGui::BeginDisabled();
        if (ImGui::Button("  DEAL  ", {160.0f, 45.0f})) {
            game->placeBet(betAmount);
            game->play();
            resultMsg.clear();
        }
        if (!canDeal) ImGui::EndDisabled();
        Theme::PopButtonGold();

        ImGui::SetCursorPos({cx - 80.0f, top + 365.0f});
        if (ImGui::Button("  Back to Lobby  ", {160.0f, 38.0f}))
            sm.changeState(std::make_unique<LobbyState>(sm, app));
    }

    // ---- Player-turn action buttons ----
    if (game->getPhase() == BlackjackPhase::PlayerTurn) {
        ImVec2 btnSize{140.0f, 40.0f};

        Theme::PushButtonGold();
        ImGui::SetCursorPos({cx - 220.0f, top + 230.0f});
        if (ImGui::Button("  HIT  ", btnSize))
            game->hit();

        ImGui::SetCursorPos({cx - 70.0f, top + 230.0f});
        if (ImGui::Button("  STAND  ", btnSize))
            game->stand();
        Theme::PopButtonGold();

        if (game->canDoubleDown()) {
            ImGui::SetCursorPos({cx + 80.0f, top + 230.0f});
            if (ImGui::Button("  DOUBLE DOWN  ", {170.0f, 40.0f}))
                game->doubleDown();
        }

        // Player bust immediate feedback
        if (isBust(game->getPlayerHand())) {
            ImGui::SetCursorPos({cx - 80.0f, top + 285.0f});
            ImGui::PushStyleColor(ImGuiCol_Text, Theme::Red());
            ImGui::Text("BUST!");
            ImGui::PopStyleColor();
        }
    }

    // ---- Dealer turn indicator ----
    if (game->getPhase() == BlackjackPhase::DealerTurn) {
        ImGui::SetCursorPos({cx - 100.0f, top + 230.0f});
        ImGui::PushStyleColor(ImGuiCol_Text, Theme::Gold());
        ImGui::Text("Dealer is playing...");
        ImGui::PopStyleColor();
    }

    // ---- Result ----
    if (gameOver && !resultMsg.empty()) {
        ImGui::SetCursorPos({cx - 200.0f, top + 230.0f});
        bool won = (game->getResult() == GameResult::Win
                 || game->getResult() == GameResult::Blackjack);
        ImGui::PushStyleColor(ImGuiCol_Text, won ? Theme::Gold() : Theme::Red());
        ImGui::SetWindowFontScale(1.4f);
        ImGui::Text("%s", resultMsg.c_str());
        ImGui::SetWindowFontScale(1.0f);
        ImGui::PopStyleColor();

        ImVec2 btnSize{170.0f, 40.0f};
        Theme::PushButtonGold();
        ImGui::SetCursorPos({cx - 180.0f, top + 295.0f});
        if (ImGui::Button("  Play Again  ", btnSize)) {
            game = std::make_unique<Blackjack>(
                *app.getPlayer(),
                std::make_unique<DealerHitSoft17>());
            resultMsg.clear();
            dealerTimer = 0.0f;
        }
        Theme::PopButtonGold();

        ImGui::SetCursorPos({cx + 10.0f, top + 295.0f});
        if (ImGui::Button("  Back to Lobby  ", btnSize))
            sm.changeState(std::make_unique<LobbyState>(sm, app));
    }

    ImGui::End();
}
