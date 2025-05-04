#include "EventLoop.h"

#include "EventHandler.h"
#include "Logger.h"

#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_scancode.h>
#include <chrono>
using namespace aion;
using namespace std::chrono;

EventLoop::EventLoop(std::stop_token* stopToken) : SubSystem(stopToken)
{
    previousKeyboardState = new bool[SDL_SCANCODE_COUNT];
    for (int i = 0; i < SDL_SCANCODE_COUNT; ++i)
    {
        previousKeyboardState[i] = false;
    }
}

EventLoop::~EventLoop()
{
}

// TODO rename this to singular or accept multiple
void aion::EventLoop::submitEvents(const Event& event)
{
    eventQueue.push(event);
}

void aion::EventLoop::init()
{
    eventLoopThread = std::thread(&EventLoop::run, this);
}

void aion::EventLoop::run()
{
    spdlog::info("Starting event loop...");

    for (auto& listener : listeners)
    {
        listener->onInit(this);
    }

    for (auto& listener : listenersRawPtrs)
    {
        listener->onInit(this);
    }

    auto lastTick = steady_clock::now();

    while (stopToken_->stop_requested() == false)
    {
        handleTickEvent(lastTick);
        handleInputEvents();

        // Sleep for a short duration to avoid busy-waiting
        std::this_thread::sleep_for(milliseconds(1));
    }

    spdlog::info("Shutting down event loop...");
}

void aion::EventLoop::shutdown()
{
    // TODO: Terminate the thread first, otherwise it won't exit from the loop
    if (eventLoopThread.joinable())
    {
        eventLoopThread.join();
    }
}

void aion::EventLoop::handleTickEvent(std::chrono::steady_clock::time_point& lastTime)
{
    // TODO: Make this configurable
    const auto tickRate = milliseconds(16); // ~60 ticks per second
    auto now = steady_clock::now();
    if (now - lastTime >= tickRate)
    {
        lastTime = now;

        Event tickEvent(Event::Type::TICK);

        // Notify listeners about the event
        for (auto& listener : listeners)
        {
            listener->onEvent(tickEvent);
        }

        for (auto& listener : listenersRawPtrs)
        {
            listener->onEvent(tickEvent);
        }
    }
}

void aion::EventLoop::handleInputEvents()
{
    int numEvents = 0;
    const bool* currentKeyboardState = SDL_GetKeyboardState(&numEvents);
    for (int i = 0; i < numEvents; ++i)
    {
        if (currentKeyboardState[i] && !previousKeyboardState[i])
        {
            KeyboardData data{i};
            Event keyDownEvent(Event::Type::KEY_DOWN, data);
            for (auto& listener : listeners)
            {
                listener->onEvent(keyDownEvent);
            }
            for (auto& listener : listenersRawPtrs)
            {
                listener->onEvent(keyDownEvent);
            }
        }
        if (!currentKeyboardState[i] && previousKeyboardState[i])
        {
            KeyboardData data{i};
            Event keyDownEvent(Event::Type::KEY_UP, data);
            for (auto& listener : listeners)
            {
                listener->onEvent(keyDownEvent);
            }
            for (auto& listener : listenersRawPtrs)
            {
                listener->onEvent(keyDownEvent);
            }
        }
        previousKeyboardState[i] = currentKeyboardState[i];
    }

    float mouseX = 0;
    float mouseY = 0;
    SDL_MouseButtonFlags currentMouseState = SDL_GetMouseState(&mouseX, &mouseY);

    if (previouseMouseX != mouseX || previouseMouseY != mouseY)
    {
        MouseMoveData data{Vec2d(mouseX, mouseY)};
        Event mouseMoveEvent(Event::Type::MOUSE_MOVE, data);
        for (auto& listener : listeners)
        {
            listener->onEvent(mouseMoveEvent);
        }
        for (auto& listener : listenersRawPtrs)
        {
            listener->onEvent(mouseMoveEvent);
        }
        previouseMouseX = mouseX;
        previouseMouseY = mouseY;
    }

    if (currentMouseState != previousMouseState)
    {
        if ((currentMouseState & SDL_BUTTON_LMASK) && !(previousMouseState & SDL_BUTTON_LMASK))
        {
            MouseClickData data{MouseClickData::Button::LEFT, Vec2d(mouseX, mouseY)};
            Event mouseClickEvent(Event::Type::MOUSE_BTN_DOWN, data);
            for (auto& listener : listeners)
            {
                listener->onEvent(mouseClickEvent);
            }
            for (auto& listener : listenersRawPtrs)
            {
                listener->onEvent(mouseClickEvent);
            }
        }

        if (!(currentMouseState & SDL_BUTTON_LMASK) && (previousMouseState & SDL_BUTTON_LMASK))
        {
            MouseClickData data{MouseClickData::Button::LEFT, Vec2d(mouseX, mouseY)};
            Event mouseClickEvent(Event::Type::MOUSE_BTN_UP, data);
            for (auto& listener : listeners)
            {
                listener->onEvent(mouseClickEvent);
            }
            for (auto& listener : listenersRawPtrs)
            {
                listener->onEvent(mouseClickEvent);
            }
        }

        if ((currentMouseState & SDL_BUTTON_RMASK) && !(previousMouseState & SDL_BUTTON_RMASK))
        {
            MouseClickData data{MouseClickData::Button::RIGHT, Vec2d(mouseX, mouseY)};
            Event mouseClickEvent(Event::Type::MOUSE_BTN_DOWN, data);
            for (auto& listener : listeners)
            {
                listener->onEvent(mouseClickEvent);
            }
            for (auto& listener : listenersRawPtrs)
            {
                listener->onEvent(mouseClickEvent);
            }
        }
        if (!(currentMouseState & SDL_BUTTON_RMASK) && (previousMouseState & SDL_BUTTON_RMASK))
        {
            MouseClickData data{MouseClickData::Button::RIGHT, Vec2d(mouseX, mouseY)};
            Event mouseClickEvent(Event::Type::MOUSE_BTN_UP, data);
            for (auto& listener : listeners)
            {
                listener->onEvent(mouseClickEvent);
            }
            for (auto& listener : listenersRawPtrs)
            {
                listener->onEvent(mouseClickEvent);
            }
        }
        previousMouseState = currentMouseState;
    }
}

void aion::EventLoop::registerListenerRawPtr(EventHandler* listener)
{
    listenersRawPtrs.push_back(listener);
}

void EventLoop::registerListener(std::unique_ptr<EventHandler> listener)
{
    listeners.push_back(std::move(listener));
}

void EventLoop::deregisterListener(EventHandler* listener)
{
    listeners.remove_if([listener](const std::unique_ptr<EventHandler>& ptr)
                        { return ptr.get() == listener; });
    listenersRawPtrs.remove(listener);
}
