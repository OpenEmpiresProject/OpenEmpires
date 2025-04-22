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
    if (e.getType() == Event::Type::TICK)
    {
        handleInputEvents();
    }
}

void aion::InputListener::handleInputEvents()
{
    // spdlog::debug("Handle input events");

    ThreadMessage* message = nullptr;
    bool alreadyHandled[static_cast<int>(Event::Type::MAX_TYPE_MARKER)] = {false};

    while (eventQueue.try_dequeue(message))
    {
        if (message->type != ThreadMessage::Type::INPUT)
        {
            spdlog::error("Invalid message type for InputListener: {}",
                          static_cast<int>(message->type));
            throw std::runtime_error("Invalid message type for InputListener.");
        }

        for (const auto& instruction : message->commandBuffer)
        {
            auto event = static_cast<SDL_Event*>(instruction);
            // TODO Handle the event
            // spdlog::debug("Received event: {}", event->type);

            if (event->type == SDL_EVENT_KEY_DOWN &&
                !alreadyHandled[static_cast<int>(Event::Type::KEY_DOWN)])
            {
                eventLoop->submitEvents(Event(Event::Type::KEY_DOWN, event));
                alreadyHandled[static_cast<int>(Event::Type::KEY_DOWN)] = true;
            }
            else if (event->type == SDL_EVENT_KEY_UP &&
                     !alreadyHandled[static_cast<int>(Event::Type::KEY_UP)])
            {
                eventLoop->submitEvents(Event(Event::Type::KEY_UP, event));
                alreadyHandled[static_cast<int>(Event::Type::KEY_UP)] = true;
            }
            else if (event->type == SDL_EVENT_MOUSE_MOTION &&
                     !alreadyHandled[static_cast<int>(Event::Type::MOUSE_MOVE)])
            {
                eventLoop->submitEvents(Event(Event::Type::MOUSE_MOVE, event));
                alreadyHandled[static_cast<int>(Event::Type::MOUSE_MOVE)] = true;
            }
            else if (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN &&
                     !alreadyHandled[static_cast<int>(Event::Type::MOUSE_BTN_DOWN)])
            {
                eventLoop->submitEvents(Event(Event::Type::MOUSE_BTN_DOWN, event));
                alreadyHandled[static_cast<int>(Event::Type::MOUSE_BTN_DOWN)] = true;
            }
            else if (event->type == SDL_EVENT_MOUSE_BUTTON_UP &&
                     !alreadyHandled[static_cast<int>(Event::Type::MOUSE_BTN_UP)])
            {
                eventLoop->submitEvents(Event(Event::Type::MOUSE_BTN_UP, event));
                alreadyHandled[static_cast<int>(Event::Type::MOUSE_BTN_UP)] = true;
            }

            ObjectPool<SDL_Event>::release(event);
        }
    }
}
