#include "InputListener.h"

#include "Logger.h"
#include "ObjectPool.h"

#include <SDL3/SDL_events.h>

using namespace aion;
using namespace utils;

void InputListener::onInit(EventLoop* el)
{
    eventLoop = el;
}

void InputListener::onExit()
{
    // Cleanup code for input listener
}

void InputListener::onEvent(const Event& e)
{
    // if (e.getType() == Event::Type::TICK)
    // {
    // }
}
