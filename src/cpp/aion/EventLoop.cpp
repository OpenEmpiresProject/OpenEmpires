#include "EventLoop.h"

#include "EventHandler.h"
#include "utils/Logger.h"

#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_scancode.h>
#include <chrono>
using namespace aion;
using namespace std::chrono;

EventLoop::EventLoop(std::stop_token* stopToken) : SubSystem(stopToken)
{
    m_previousKeyboardState = new bool[SDL_SCANCODE_COUNT];
    for (int i = 0; i < SDL_SCANCODE_COUNT; ++i)
    {
        m_previousKeyboardState[i] = false;
    }
}

EventLoop::~EventLoop()
{
}

// TODO rename this to singular or accept multiple
void EventLoop::submitEvents(const Event& event)
{
    m_eventQueue.push(event);
}

void EventLoop::init()
{
    m_eventLoopThread = std::thread(&EventLoop::run, this);
}

void EventLoop::run()
{
    spdlog::info("Starting event loop...");

    for (auto& listener : m_listeners)
    {
        listener->onInit(this);
    }

    auto lastTick = steady_clock::now();

    while (m_stopToken->stop_requested() == false)
    {
        handleTickEvent(lastTick);
        handleInputEvents();

        // Sleep for a short duration to avoid busy-waiting
        std::this_thread::sleep_for(milliseconds(1));
    }

    spdlog::info("Shutting down event loop...");
}

void EventLoop::shutdown()
{
    // TODO: Terminate the thread first, otherwise it won't exit from the loop
    if (m_eventLoopThread.joinable())
    {
        m_eventLoopThread.join();
    }
}

void EventLoop::handleTickEvent(std::chrono::steady_clock::time_point& lastTime)
{
    // TODO: Make this configurable
    const auto tickRate = milliseconds(16); // ~60 ticks per second
    auto now = steady_clock::now();
    if (now - lastTime >= tickRate)
    {
        lastTime = now;

        Event tickEvent(Event::Type::TICK);

        // Notify listeners about the event
        for (auto& listener : m_listeners)
        {
            listener->onEvent(tickEvent);
        }
    }
}

void EventLoop::handleInputEvents()
{
    int numEvents = 0;
    const bool* currentKeyboardState = SDL_GetKeyboardState(&numEvents);
    for (int i = 0; i < numEvents; ++i)
    {
        if (currentKeyboardState[i] && !m_previousKeyboardState[i])
        {
            KeyboardData data{i};
            Event keyDownEvent(Event::Type::KEY_DOWN, data);
            for (auto& listener : m_listeners)
            {
                listener->onEvent(keyDownEvent);
            }
        }
        if (!currentKeyboardState[i] && m_previousKeyboardState[i])
        {
            KeyboardData data{i};
            Event keyDownEvent(Event::Type::KEY_UP, data);
            for (auto& listener : m_listeners)
            {
                listener->onEvent(keyDownEvent);
            }
        }
        m_previousKeyboardState[i] = currentKeyboardState[i];
    }

    float mouseX = 0;
    float mouseY = 0;
    SDL_MouseButtonFlags currentMouseState = SDL_GetMouseState(&mouseX, &mouseY);

    if (m_previouseMouseX != mouseX || m_previouseMouseY != mouseY)
    {
        MouseMoveData data{Vec2d(mouseX, mouseY)};
        Event mouseMoveEvent(Event::Type::MOUSE_MOVE, data);
        for (auto& listener : m_listeners)
        {
            listener->onEvent(mouseMoveEvent);
        }
        m_previouseMouseX = mouseX;
        m_previouseMouseY = mouseY;
    }

    if (currentMouseState != m_previousMouseState)
    {
        if ((currentMouseState & SDL_BUTTON_LMASK) && !(m_previousMouseState & SDL_BUTTON_LMASK))
        {
            MouseClickData data{MouseClickData::Button::LEFT, Vec2d(mouseX, mouseY)};
            Event mouseClickEvent(Event::Type::MOUSE_BTN_DOWN, data);
            for (auto& listener : m_listeners)
            {
                listener->onEvent(mouseClickEvent);
            }
        }

        if (!(currentMouseState & SDL_BUTTON_LMASK) && (m_previousMouseState & SDL_BUTTON_LMASK))
        {
            MouseClickData data{MouseClickData::Button::LEFT, Vec2d(mouseX, mouseY)};
            Event mouseClickEvent(Event::Type::MOUSE_BTN_UP, data);
            for (auto& listener : m_listeners)
            {
                listener->onEvent(mouseClickEvent);
            }
        }

        if ((currentMouseState & SDL_BUTTON_RMASK) && !(m_previousMouseState & SDL_BUTTON_RMASK))
        {
            MouseClickData data{MouseClickData::Button::RIGHT, Vec2d(mouseX, mouseY)};
            Event mouseClickEvent(Event::Type::MOUSE_BTN_DOWN, data);
            for (auto& listener : m_listeners)
            {
                listener->onEvent(mouseClickEvent);
            }
        }
        if (!(currentMouseState & SDL_BUTTON_RMASK) && (m_previousMouseState & SDL_BUTTON_RMASK))
        {
            MouseClickData data{MouseClickData::Button::RIGHT, Vec2d(mouseX, mouseY)};
            Event mouseClickEvent(Event::Type::MOUSE_BTN_UP, data);
            for (auto& listener : m_listeners)
            {
                listener->onEvent(mouseClickEvent);
            }
        }
        m_previousMouseState = currentMouseState;
    }
}

void EventLoop::registerListener(std::shared_ptr<EventHandler> listener)
{
    m_listeners.push_back(std::move(listener));
}
