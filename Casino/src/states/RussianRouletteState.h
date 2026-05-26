#pragma once
#include "../core/IState.h"
#include "../games/RussianRoulette.h"
#include <memory>
#include <string>

enum class RRPhase { Setup, Pulling, Result };

class RussianRouletteState : public IState
{
public:
    RussianRouletteState(StateManager& sm, Application& app);
    void onEnter() override;
    void update(float dt) override;
    void render() override;

private:
    std::unique_ptr<RussianRoulette> game;
    RRPhase phase = RRPhase::Setup;

    float betAmount      = 10.0f;
    float pullTimer      = 0.0f;
    float shakeTimer     = 0.0f;
    float cylinderAngle  = 0.0f;   // animation spin
    std::string resultMsg;

    void renderCylinder(float screenX, float screenY);
    void renderSetup(float cx, float cy);
    void renderResult(float cx, float cy);
};
