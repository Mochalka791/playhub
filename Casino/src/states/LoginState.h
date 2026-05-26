#pragma once
#include "../core/IState.h"
#include <array>
#include <string>

class LoginState : public IState
{
public:
    LoginState(StateManager& sm, Application& app);
    void onEnter() override;
    void update(float dt) override {}
    void render() override;

private:
    std::array<char, 64> nameBuf{};
    float startBalance = 1000.0f;
    std::string errorMsg;
};
