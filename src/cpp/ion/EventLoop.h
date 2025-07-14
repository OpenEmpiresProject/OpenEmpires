#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include "Event.h"
#include "EventPublisher.h"
#include "SubSystem.h"

#include <list>
#include <memory>
#include <queue>
#include <thread>

namespace ion
{
class EventHandler;
class EventLoop : public SubSystem, public EventPublisher
{
  public:
    EventLoop(std::stop_token* stopToken);
    ~EventLoop();

    void registerListener(std::shared_ptr<EventHandler> listener);
    int getListenersCount() const
    {
        return m_listeners.size();
    }

    bool isReady() const
    {
        return m_isReady;
    }

    static bool isPaused()
    {
        return s_isPaused;
    }

    static void setPaused(bool isPaused)
    {
        s_isPaused = isPaused;
    }

  private:
    // SubSystem methods
    void init() override;
    void run();
    void shutdown() override;
    void handleTickEvent(std::chrono::steady_clock::time_point& lastTime);
    void handleInputEvents();
    void handleGameEvents();
    void publish(const Event& event) override;

    std::list<std::shared_ptr<EventHandler>> m_listeners;
    std::thread m_eventLoopThread;
    std::queue<Event> m_eventQueue;

    bool* m_previousKeyboardState = nullptr;
    uint32_t m_previousMouseState = 0;
    int m_previouseMouseX = 0;
    int m_previouseMouseY = 0;

    static bool s_isPaused;
    bool m_isReady = false;
};

} // namespace ion

#endif // EVENTLOOP_H