#pragma once
#include "../core/IState.h"
#include <string>

class LobbyState : public IState
{
public:
    LobbyState(StateManager& sm, Application& app);
    void onEnter() override;
    void update(float dt) override;
    void render() override;

private:
    std::string dailyBonusMsg;
    float       dailyBonusTimer = 0.0f;
    std::string achieveMsg;
    float       achieveTimer    = 0.0f;
};
