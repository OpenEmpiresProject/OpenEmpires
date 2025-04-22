#include "EventLoop.h"

#include "EventHandler.h"
#include "Logger.h"

#include <chrono>

using namespace aion;
using namespace std::chrono;

EventLoop::EventLoop(std::stop_token* stopToken) : SubSystem(stopToken)
{
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
    while (!eventQueue.empty())
    {
        auto event = eventQueue.front();
        eventQueue.pop();
        for (auto& listener : listeners)
        {
            listener->onEvent(event);
        }

        for (auto& listener : listenersRawPtrs)
        {
            listener->onEvent(event);
        }
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
