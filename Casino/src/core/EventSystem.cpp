#include "EventSystem.h"

EventBus& EventBus::instance()
{
    static EventBus bus;
    return bus;
}

void EventBus::clear()
{
    handlers.clear();
}
