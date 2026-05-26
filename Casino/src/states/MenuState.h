#pragma once
#include "../core/IState.h"

class MenuState : public IState
{
public:
    MenuState(StateManager& sm, Application& app);
    void onEnter() override;
    void update(float dt) override {}
    void render() override;
};
