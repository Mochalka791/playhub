#pragma once
#include "../core/IState.h"
#include "../games/Blackjack.h"
#include "../ui/ParticleSystem.h"
#include <memory>
#include <string>

class BlackjackState : public IState
{
public:
    BlackjackState(StateManager& sm, Application& app);
    void onEnter() override;
    void update(float dt) override;
    void render() override;

private:
    std::unique_ptr<Blackjack> game;

    float betAmount = 10.0f;
    std::string resultMsg;

    // Dealer-turn auto-play timer
    float dealerTimer = 0.0f;
    static constexpr float DEALER_STEP_DELAY = 0.8f;

    ParticleSystem particles;
    bool resultResolved = false;

    void renderHand(ImDrawList* draw, ImVec2 winPos, const Hand& hand,
                    float startX, float y, bool hideFirst) const;
    static void drawCard(ImDrawList* draw, ImVec2 tl, const Card& card, bool faceDown = false);
    void resolveAndShowResult();
};
