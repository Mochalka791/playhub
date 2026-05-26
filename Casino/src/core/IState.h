#pragma once

class StateManager;
class Application;

class IState
{
public:
    IState(StateManager& sm, Application& app) : sm(sm), app(app) {}
    virtual ~IState() = default;

    virtual void onEnter() {}
    virtual void onExit() {}
    virtual void update(float dt) = 0;
    virtual void render() = 0;

protected:
    StateManager& sm;
    Application& app;
};
