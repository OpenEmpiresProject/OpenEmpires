#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include "Event.h"
#include "EventPublisher.h"
#include "InputProcessor.h"
#include "SubSystem.h"

#include <list>
#include <memory>
#include <queue>
#include <thread>

namespace core
{
class EventHandler;
class EventLoop : public SubSystem,
                  public EventPublisher,
                  public std::enable_shared_from_this<EventLoop>
{
  public:
    EventLoop(std::stop_token* stopToken);

    void registerListener(std::shared_ptr<EventHandler> listener);
    void registerListener(std::shared_ptr<EventHandler> listener, bool immuneToPause);
    inline int getListenersCount() const
    {
        return m_listeners.size();
    }

    inline bool isReady() const
    {
        return m_isReady;
    }

    bool isPaused() const
    {
        return s_isPaused;
    }

    bool isTemporarilyUnpaused() const
    {
        return s_isPaused and m_framesRemainingToPlay > 0;
    }

    void setPaused(bool isPaused)
    {
        s_isPaused = isPaused;
    }

    void setFramesRemainingToPlay(int count)
    {
        m_framesRemainingToPlay = count;
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
    std::list<Ref<EventHandler>> m_listeners;
    std::list<Ref<EventHandler>> m_immunedListeners;
    std::thread m_eventLoopThread;
    std::queue<Event> m_eventQueue;

    InputProcessor m_inputProcessor;

    bool s_isPaused = false;
    int m_framesRemainingToPlay = 0;
    bool m_isReady = false;

    int m_currentTick = 0;
};

} // namespace core

#endif // EVENTLOOP_H