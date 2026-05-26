#pragma once
#include <functional>
#include <typeindex>
#include <unordered_map>
#include <vector>
#include <memory>
#include <string>

// ---- Event types ----------------------------------------

struct WinEvent          { std::string playerName; double amount; std::string game; };
struct LossEvent         { std::string playerName; double amount; std::string game; };
struct GameStartEvent    { std::string playerName; std::string game; };
struct GameEndEvent      { std::string playerName; std::string game; double netChange; };
struct BalanceChangedEvent { std::string playerName; double newBalance; };

// ---- Handler base ---------------------------------------

struct IEventHandler {
    virtual ~IEventHandler() = default;
};

template<typename T>
struct TypedEventHandler : IEventHandler {
    std::function<void(const T&)> callback;
    explicit TypedEventHandler(std::function<void(const T&)> cb)
        : callback(std::move(cb)) {}
};

// ---- EventBus -------------------------------------------

class EventBus
{
public:
    static EventBus& instance();

    template<typename T>
    void subscribe(std::function<void(const T&)> callback)
    {
        handlers[std::type_index(typeid(T))].push_back(
            std::make_shared<TypedEventHandler<T>>(std::move(callback)));
    }

    template<typename T>
    void publish(const T& event)
    {
        auto it = handlers.find(std::type_index(typeid(T)));
        if (it == handlers.end()) return;
        for (auto& h : it->second) {
            auto typed = std::dynamic_pointer_cast<TypedEventHandler<T>>(h);
            if (typed) typed->callback(event);
        }
    }

    void clear();

private:
    EventBus() = default;
    std::unordered_map<std::type_index,
        std::vector<std::shared_ptr<IEventHandler>>> handlers;
};
