#include "StateManager.h"

void StateManager::pushState(std::unique_ptr<IState> state)
{
    pending.push_back({ ActionType::Push, std::move(state) });
}

void StateManager::popState()
{
    pending.push_back({ ActionType::Pop, nullptr });
}

void StateManager::changeState(std::unique_ptr<IState> state)
{
    pending.push_back({ ActionType::Change, std::move(state) });
}

void StateManager::update(float dt)
{
    if (!stack.empty())
        stack.back()->update(dt);

    applyPendingChanges();
}

void StateManager::render()
{
    if (!stack.empty())
        stack.back()->render();
}

bool StateManager::isEmpty() const
{
    return stack.empty();
}

void StateManager::applyPendingChanges()
{
    for (auto& action : pending) {
        switch (action.type) {
        case ActionType::Push:
            action.state->onEnter();
            stack.push_back(std::move(action.state));
            break;

        case ActionType::Pop:
            if (!stack.empty()) {
                stack.back()->onExit();
                stack.pop_back();
                if (!stack.empty())
                    stack.back()->onEnter();
            }
            break;

        case ActionType::Change:
            if (!stack.empty()) {
                stack.back()->onExit();
                stack.pop_back();
            }
            action.state->onEnter();
            stack.push_back(std::move(action.state));
            break;
        }
    }
    pending.clear();
}
