#pragma once
#include "../core/IState.h"

class SettingsState : public IState
{
public:
    SettingsState(StateManager& sm, Application& app);
    void onEnter() override;
    void update(float dt) override {}
    void render() override;

private:
    // Local copies edited before saving
    int   volume         = 100;
    bool  fullscreen     = false;
    int   dealerStrategy = 0;
    float minBet         = 1.0f;
    float maxBet         = 10000.0f;
};
