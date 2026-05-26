#pragma once
#include "../core/IState.h"

class LobbyState : public IState
{
public:
    LobbyState(StateManager& sm, Application& app);
    void onEnter() override;
    void update(float dt) override {}
    void render() override;
};
