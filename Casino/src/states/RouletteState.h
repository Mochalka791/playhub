#pragma once
#include "../core/IState.h"
#include "../games/Roulette.h"
#include "../ui/AnimationSystem.h"
#include "../ui/ParticleSystem.h"
#include <memory>
#include <string>

class RouletteState : public IState
{
public:
    RouletteState(StateManager& sm, Application& app);
    void onEnter() override;
    void update(float dt) override;
    void render() override;

private:
    std::unique_ptr<Roulette> game;
    RouletteAnimation spinAnim;

    float betAmount      = 10.0f;
    int   straightNum    = 0;
    std::string resultMsg;
    bool  showResult     = false;

    // Wheel animation
    float wheelAngle     = 0.0f;  // current rotation (radians)
    float spinStartAngle = 0.0f;
    float totalSpinAmt   = 0.0f;
    int   finalNumber    = -1;

    ParticleSystem particles;
    bool  spinSoundPlayed = false;
    bool  resultSoundPlayed = false;

    void renderWheel(float screenX, float screenY, float radius);
    void renderBettingPanel(float panelX, float panelY);
    int  findWheelIndex(int number) const;
};
