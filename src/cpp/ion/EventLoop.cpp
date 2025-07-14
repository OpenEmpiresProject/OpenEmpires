#include "EventLoop.h"

#include "EventHandler.h"
#include "EventPublisher.h"
#include "utils/Logger.h"

#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_scancode.h>
#include <chrono>
using namespace ion;
using namespace std::chrono;

bool EventLoop::s_isPaused = false;

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

void EventLoop::init()
{
    m_eventLoopThread = std::thread(&EventLoop::run, this);
}

void EventLoop::run()
{
    spdlog::info("Starting event loop...");

    registerPublisher();

    for (auto& listener : m_listeners)
    {
        listener->onInit(this);
    }

    auto lastTick = steady_clock::now();

    while (m_stopToken->stop_requested() == false)
    {
        if (!isPaused())
        {
            handleInputEvents();
            handleTickEvent(lastTick);
            handleGameEvents();
        }
        else
        {
            std::this_thread::sleep_for(milliseconds(100));
        }

        // Sleep for a short duration to avoid busy-waiting
        std::this_thread::sleep_for(milliseconds(1));
        m_isReady = true;
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
    const auto tickRate = milliseconds(1000 / Constants::FIXED_FPS);
    // Don't jump more than a fixed number of milliseconds. Useful when unpause the simulation
    // where the actual time would be moved by a lot.
    const auto maxDelay = milliseconds(Constants::MAX_FRAME_DELAY_MS);
    auto now = steady_clock::now();
    if (now - lastTime >= tickRate)
    {
        auto duration = now - lastTime;
        if (duration > maxDelay)
            duration = tickRate;

        TickData data{static_cast<int>(duration_cast<milliseconds>(duration).count())};
        Event tickEvent(Event::Type::TICK, data);

        // Notify listeners about the event
        for (auto& listener : m_listeners)
        {
            listener->dispatchEvent(tickEvent);
        }
        lastTime = now;
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
                listener->dispatchEvent(keyDownEvent);
            }
        }
        if (!currentKeyboardState[i] && m_previousKeyboardState[i])
        {
            KeyboardData data{i};
            Event keyDownEvent(Event::Type::KEY_UP, data);
            for (auto& listener : m_listeners)
            {
                listener->dispatchEvent(keyDownEvent);
            }
        }
        m_previousKeyboardState[i] = currentKeyboardState[i];
    }

    float mouseX = 0;
    float mouseY = 0;
    SDL_MouseButtonFlags currentMouseState = SDL_GetMouseState(&mouseX, &mouseY);

    if (m_previouseMouseX != mouseX || m_previouseMouseY != mouseY)
    {
        MouseMoveData data{Vec2(mouseX, mouseY)};
        Event mouseMoveEvent(Event::Type::MOUSE_MOVE, data);
        for (auto& listener : m_listeners)
        {
            listener->dispatchEvent(mouseMoveEvent);
        }
        m_previouseMouseX = mouseX;
        m_previouseMouseY = mouseY;
    }

    if (currentMouseState != m_previousMouseState)
    {
        if ((currentMouseState & SDL_BUTTON_LMASK) && !(m_previousMouseState & SDL_BUTTON_LMASK))
        {
            MouseClickData data{MouseClickData::Button::LEFT, Vec2(mouseX, mouseY)};
            Event mouseClickEvent(Event::Type::MOUSE_BTN_DOWN, data);
            for (auto& listener : m_listeners)
            {
                listener->dispatchEvent(mouseClickEvent);
            }
        }

        if (!(currentMouseState & SDL_BUTTON_LMASK) && (m_previousMouseState & SDL_BUTTON_LMASK))
        {
            MouseClickData data{MouseClickData::Button::LEFT, Vec2(mouseX, mouseY)};
            Event mouseClickEvent(Event::Type::MOUSE_BTN_UP, data);
            for (auto& listener : m_listeners)
            {
                listener->dispatchEvent(mouseClickEvent);
            }
        }

        if ((currentMouseState & SDL_BUTTON_RMASK) && !(m_previousMouseState & SDL_BUTTON_RMASK))
        {
            MouseClickData data{MouseClickData::Button::RIGHT, Vec2(mouseX, mouseY)};
            Event mouseClickEvent(Event::Type::MOUSE_BTN_DOWN, data);
            for (auto& listener : m_listeners)
            {
                listener->dispatchEvent(mouseClickEvent);
            }
        }
        if (!(currentMouseState & SDL_BUTTON_RMASK) && (m_previousMouseState & SDL_BUTTON_RMASK))
        {
            MouseClickData data{MouseClickData::Button::RIGHT, Vec2(mouseX, mouseY)};
            Event mouseClickEvent(Event::Type::MOUSE_BTN_UP, data);
            for (auto& listener : m_listeners)
            {
                listener->dispatchEvent(mouseClickEvent);
            }
        }
        m_previousMouseState = currentMouseState;
    }
}

void EventLoop::handleGameEvents()
{
    while (!m_eventQueue.empty())
    {
        auto& event = m_eventQueue.front();
        for (auto& listener : m_listeners)
        {
            listener->dispatchEvent(event);
        }
        m_eventQueue.pop();
    }
}

void EventLoop::publish(const Event& event)
{
    m_eventQueue.push(event);
}

void EventLoop::registerListener(std::shared_ptr<EventHandler> listener)
{
    m_listeners.push_back(std::move(listener));
}
