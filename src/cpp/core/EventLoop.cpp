#include "EventLoop.h"

#include "EventHandler.h"
#include "EventPublisher.h"
#include "logging/Logger.h"

#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_mouse.h>
#include <SDL3/SDL_scancode.h>
#include <chrono>
using namespace core;
using namespace std::chrono;

EventLoop::EventLoop(std::stop_token* stopToken) : SubSystem(stopToken)
{
}

void EventLoop::init()
{
    m_eventLoopThread = std::thread(&EventLoop::run, this);
}

void EventLoop::run()
{
    spdlog::info("Starting event loop...");

    registerPublisher(shared_from_this());
    auto thisRef = shared_from_this();

    for (auto& listener : m_listeners)
    {
        listener->onInit(*this);
        listener->onInit(thisRef);
    }

    auto lastTick = steady_clock::now();

    while (m_stopToken->stop_requested() == false)
    {
        handleInputEvents();
        handleTickEvent(lastTick);

        if (not isPaused() or isTemporarilyUnpaused())
        {
            m_framesRemainingToPlay--;
            handleGameEvents();
        }
        else
        {
            // If the simulation is paused, go with fixed 60FPS
            std::this_thread::sleep_for(milliseconds(16));
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

void EventLoop::handleInputEvents()
{
    auto isRunning = not isPaused() or isTemporarilyUnpaused();
    auto& listners = isRunning ? m_listeners : m_immunedListeners;

    m_inputProcessor.processInputs(
        [this, &listners](const Event& e)
        {
            for (auto& listener : listners)
            {
                bool consumed = listener->dispatchEvent(e);
                if (consumed)
                    break;
            }
        });
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
        auto isRunning = not isPaused() or isTemporarilyUnpaused();
        auto& listners = isRunning ? m_listeners : m_immunedListeners;

        auto duration = now - lastTime;
        if (duration > maxDelay)
            duration = tickRate;

        ++m_currentTick;
        auto deltaMs = static_cast<int>(duration_cast<milliseconds>(duration).count());

        TickData data{.deltaTimeMs = deltaMs, .currentTick = m_currentTick};
        Event tickEvent(Event::Type::TICK, data);

        // Notify listeners about the event
        for (auto& listener : listners)
        {
            bool consumed = listener->dispatchEvent(tickEvent);
            if (consumed)
                break;
        }
        lastTime = now;
    }
}

void EventLoop::handleGameEvents()
{
    while (!m_eventQueue.empty())
    {
        auto& event = m_eventQueue.front();
        for (auto& listener : m_listeners)
        {
            bool consumed = listener->dispatchEvent(event);
            if (consumed)
                break;
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

void EventLoop::registerListener(std::shared_ptr<EventHandler> listener, bool immuneToPause)
{
    if (immuneToPause)
    {
        m_immunedListeners.push_back(listener);
    }
    m_listeners.push_back(std::move(listener));
}
