#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include "Event.h"
#include "EventPublisher.h"
#include "SubSystem.h"

#include <list>
#include <memory>
#include <queue>
#include <thread>

namespace core
{
class EventHandler;
class EventLoop : public SubSystem, public EventPublisher
{
  public:
    EventLoop(std::stop_token* stopToken);

    void registerListener(std::shared_ptr<EventHandler> listener);
    inline int getListenersCount() const
    {
        return m_listeners.size();
    }

    inline bool isReady() const
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
    void shutdown() override;
    // EventPublisher methods
    void publish(const Event& event) override;

    void run();
    void handleTickEvent(std::chrono::steady_clock::time_point& lastTime);
    void handleInputEvents();
    void handleGameEvents();

  private:
    std::list<std::shared_ptr<EventHandler>> m_listeners;
    std::thread m_eventLoopThread;
    std::queue<Event> m_eventQueue;

    bool* m_previousKeyboardState = nullptr;
    uint32_t m_previousMouseState = 0;
    int m_previouseMouseX = 0;
    int m_previouseMouseY = 0;

    static bool s_isPaused;
    bool m_isReady = false;

    int m_currentTick = 0;
};

} // namespace core

#endif // EVENTLOOP_H