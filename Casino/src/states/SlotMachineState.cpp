#include "SlotMachineState.h"
#include "LobbyState.h"
#include "../core/StateManager.h"
#include "../core/Application.h"
#include "../models/Player.h"
#include "../ui/Theme.h"
#include <imgui.h>
#include <random>
#include <string>

SlotMachineState::SlotMachineState(StateManager& sm, Application& app)
    : IState(sm, app) {}

void SlotMachineState::onEnter()
{
    game = std::make_unique<SlotMachine>(*app.getPlayer());
    betAmount  = 10.0f;
    resultMsg.clear();
    showResult = false;
    reelAnim   = {};
    displayReels.fill(SlotSymbol::Seven);
}

void SlotMachineState::update(float dt)
{
    if (reelAnim.active) {
        reelAnim.update(dt);
        updateDisplayReels();

        if (reelAnim.finished() && !showResult) {
            // Lock in the final reel results
            displayReels = game->getReels();
            showResult   = true;

            double payout = game->getPayout();
            GameResult res = game->getResult();
            if (res == GameResult::Win)
                resultMsg = "YOU WIN!  Payout: $" + std::to_string((int)payout);
            else if (res == GameResult::Push)
                resultMsg = "Two matching! Bet returned.";
            else
                resultMsg = "No match. Better luck next time!";
        }
    }
}

void SlotMachineState::updateDisplayReels()
{
    // Only randomize reels that haven't stopped yet
    randomizeDisplayReels(
        !reelAnim.reel1Done(),
        !reelAnim.reel2Done(),
        !reelAnim.reel3Done());

    // Once a reel stops, show final value
    const auto& final = game->getReels();
    if (reelAnim.reel1Done()) displayReels[0] = final[0];
    if (reelAnim.reel2Done()) displayReels[1] = final[1];
    if (reelAnim.reel3Done()) displayReels[2] = final[2];
}

void SlotMachineState::randomizeDisplayReels(bool r1, bool r2, bool r3)
{
    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(0, static_cast<int>(SlotSymbol::COUNT) - 1);
    if (r1) displayReels[0] = static_cast<SlotSymbol>(dist(rng));
    if (r2) displayReels[1] = static_cast<SlotSymbol>(dist(rng));
    if (r3) displayReels[2] = static_cast<SlotSymbol>(dist(rng));
}

void SlotMachineState::render()
{
    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos({0, 0});
    ImGui::SetNextWindowSize(io.DisplaySize);
    ImGui::Begin("##slots", nullptr,
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoScrollbar  | ImGuiWindowFlags_NoSavedSettings);

    float cx  = io.DisplaySize.x * 0.5f;
    float top = 30.0f;

    // Title
    Theme::PushTitleStyle();
    ImGui::SetWindowFontScale(1.8f);
    ImGui::SetCursorPos({cx - 120.0f, top});
    ImGui::Text("SLOT MACHINE");
    ImGui::SetWindowFontScale(1.0f);
    Theme::PopTitleStyle();

    // Balance
    if (Player* p = app.getPlayer()) {
        ImGui::SetCursorPos({cx - 160.0f, top + 55.0f});
        ImGui::PushStyleColor(ImGuiCol_Text, Theme::Gold());
        ImGui::Text("Balance: $%.2f", p->getBalance());
        ImGui::PopStyleColor();
    }

    // Reel display box
    ImGui::SetCursorPos({cx - 200.0f, top + 95.0f});
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4{0.04f, 0.18f, 0.04f, 1.0f});
    ImGui::BeginChild("##reels", {400.0f, 80.0f}, true);

    // Per-symbol colours
    auto symColor = [](SlotSymbol s, bool stopped) -> ImVec4 {
        if (!stopped) return {0.7f, 0.7f, 0.7f, 0.5f};
        switch (s) {
            case SlotSymbol::Crown:   return {1.00f, 0.84f, 0.00f, 1.0f}; // gold
            case SlotSymbol::Seven:   return {0.95f, 0.15f, 0.15f, 1.0f}; // red
            case SlotSymbol::Diamond: return {0.40f, 0.80f, 1.00f, 1.0f}; // cyan
            case SlotSymbol::Bar:     return {0.90f, 0.90f, 0.90f, 1.0f}; // white
            case SlotSymbol::Bell:    return {1.00f, 0.90f, 0.20f, 1.0f}; // yellow
            case SlotSymbol::Star:    return {1.00f, 0.85f, 0.20f, 1.0f}; // yellow
            case SlotSymbol::Cherry:  return {1.00f, 0.40f, 0.55f, 1.0f}; // pink
            case SlotSymbol::Lemon:   return {0.95f, 0.95f, 0.20f, 1.0f}; // yellow
            case SlotSymbol::Orange:  return {1.00f, 0.55f, 0.10f, 1.0f}; // orange
            case SlotSymbol::Clover:  return {0.20f, 0.85f, 0.30f, 1.0f}; // green
            default:                  return {0.80f, 0.80f, 0.80f, 1.0f};
        }
    };

    ImGui::SetWindowFontScale(2.0f);
    float reelX = 10.0f;
    for (int i = 0; i < 3; ++i) {
        ImGui::SetCursorPos({reelX, 12.0f});
        bool isReel1Done = reelAnim.reel1Done() || !reelAnim.active;
        bool isReel2Done = reelAnim.reel2Done() || !reelAnim.active;
        bool isReel3Done = reelAnim.reel3Done() || !reelAnim.active;
        bool stopped = (i == 0 && isReel1Done) || (i == 1 && isReel2Done) || (i == 2 && isReel3Done);
        ImGui::PushStyleColor(ImGuiCol_Text, symColor(displayReels[i], stopped));
        ImGui::Text("[%s]", SlotMachine::symbolToString(displayReels[i]).c_str());
        ImGui::PopStyleColor();
        reelX += 130.0f;
    }
    ImGui::SetWindowFontScale(1.0f);

    ImGui::EndChild();
    ImGui::PopStyleColor();

    // Bet amount
    ImGui::SetCursorPos({cx - 200.0f, top + 190.0f});
    ImGui::Text("Bet Amount:");
    ImGui::SetCursorPos({cx - 200.0f, top + 215.0f});
    ImGui::SetNextItemWidth(300.0f);
    float maxBet = app.getPlayer() ? (float)app.getPlayer()->getBalance() : 100.0f;
    ImGui::SliderFloat("##bet", &betAmount, 1.0f, std::max(1.0f, maxBet), "%.0f $");

    // Spin button
    bool canSpin = !reelAnim.active && !showResult
                && app.getPlayer() && app.getPlayer()->canBet(betAmount);

    ImGui::SetCursorPos({cx - 80.0f, top + 265.0f});
    Theme::PushButtonGold();
    if (!canSpin) ImGui::BeginDisabled();
    if (ImGui::Button("   SPIN!   ", {160.0f, 50.0f})) {
        game->placeBet(betAmount);
        game->play();
        reelAnim.start();
        showResult = false;
        resultMsg.clear();
        displayReels.fill(SlotSymbol::Seven);
    }
    if (!canSpin) ImGui::EndDisabled();
    Theme::PopButtonGold();

    // Spinning indicator
    if (reelAnim.active) {
        ImGui::SetCursorPos({cx - 60.0f, top + 330.0f});
        ImGui::PushStyleColor(ImGuiCol_Text, Theme::Gold());
        int dots = (reelAnim.frameCounter / 15) % 4;
        const char* anim[] = {"Spinning.", "Spinning..", "Spinning...", "Spinning   "};
        ImGui::Text("%s", anim[dots]);
        ImGui::PopStyleColor();
    }

    // Result message
    if (showResult && !reelAnim.active) {
        ImGui::SetCursorPos({cx - 180.0f, top + 335.0f});
        bool won = game->getResult() == GameResult::Win;
        ImGui::PushStyleColor(ImGuiCol_Text, won ? Theme::Gold() : (game->getResult() == GameResult::Push ? ImVec4{0.8f,0.8f,0.2f,1.0f} : Theme::Red()));
        ImGui::Text("%s", resultMsg.c_str());
        ImGui::PopStyleColor();

        ImVec2 btnSize{170.0f, 38.0f};
        Theme::PushButtonGold();
        ImGui::SetCursorPos({cx - 180.0f, top + 380.0f});
        if (ImGui::Button("  Play Again  ", btnSize)) {
            game = std::make_unique<SlotMachine>(*app.getPlayer());
            showResult = false;
            resultMsg.clear();
        }
        Theme::PopButtonGold();

        ImGui::SetCursorPos({cx + 10.0f, top + 380.0f});
        if (ImGui::Button("  Back to Lobby  ", btnSize))
            sm.changeState(std::make_unique<LobbyState>(sm, app));
    }

    // Paytable (two lines)
    ImGui::SetCursorPos({cx - 260.0f, io.DisplaySize.y - 115.0f});
    ImGui::PushStyleColor(ImGuiCol_Text, Theme::DarkGold());
    ImGui::Text("Paytable:  CROWN=20x  7=12x  DIA=10x  BAR=6x  BEL=5x");
    ImGui::SetCursorPos({cx - 260.0f, io.DisplaySize.y - 90.0f});
    ImGui::Text("           STR/CHRRY=4x  LEMN/ORNG=3x  CLVR=2x  |  2x same=bet back");
    ImGui::PopStyleColor();

    ImGui::End();
}
