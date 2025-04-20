#include "EventLoop.h"

#include "Logger.h"

#include <chrono>

using namespace aion;
using namespace std::chrono;

EventLoop::EventLoop(std::stop_token *stopToken) : SubSystem(stopToken) {}

EventLoop::~EventLoop() {}

void aion::EventLoop::init() { eventLoopThread = std::thread(&EventLoop::run, this); }

void aion::EventLoop::run()
{
    spdlog::info("Starting event loop...");

    auto lastTick = steady_clock::now();
    // TODO: Make this configurable
    const auto tickRate = milliseconds(16); // ~60 ticks per second

    while (stopToken_->stop_requested() == false)
    {
        auto now = steady_clock::now();
        if (now - lastTick >= tickRate)
        {
            lastTick = now;

            Event tickEvent(Event::Type::TICK);

            // Notify listeners about the event
            for (auto &listener : listeners)
            {
                listener->onEvent(tickEvent);
            }
        }

        // Sleep for a short duration to avoid busy-waiting
        std::this_thread::sleep_for(milliseconds(1));
    }
}

void aion::EventLoop::shutdown()
{
    // TODO: Terminate the thread first, otherwise it won't exit from the loop
    if (eventLoopThread.joinable())
    {
        eventLoopThread.join();
    }
}

void EventLoop::registerListener(std::unique_ptr<EventListener> listener)
{
    listeners.push_back(std::move(listener));
}

void EventLoop::deregisterListener(EventListener *listener)
{
    listeners.remove_if([listener](const std::unique_ptr<EventListener> &ptr)
                        { return ptr.get() == listener; });
}
