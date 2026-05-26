#pragma once
#include "../core/IState.h"

class HighscoreState : public IState
{
public:
    HighscoreState(StateManager& sm, Application& app);
    void onEnter() override {}
    void update(float dt) override {}
    void render() override;
};
