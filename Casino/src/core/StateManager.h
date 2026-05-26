#pragma once
#include "IState.h"
#include <vector>
#include <memory>
#include <functional>

class StateManager
{
public:
    // Queues a state transition (applied at end of current frame)
    void pushState(std::unique_ptr<IState> state);
    void popState();
    void changeState(std::unique_ptr<IState> state);

    void update(float dt);
    void render();
    bool isEmpty() const;

    // Applies queued transitions
    void applyPendingChanges();

private:
    std::vector<std::unique_ptr<IState>> stack;

    enum class ActionType { Push, Pop, Change };
    struct PendingAction {
        ActionType type;
        std::unique_ptr<IState> state;   // valid for Push / Change
    };
    std::vector<PendingAction> pending;
};
