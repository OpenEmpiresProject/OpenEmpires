#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include "Event.h"
#include "SubSystem.h"

#include <list>
#include <memory>
#include <queue>
#include <thread>

namespace aion
{
class EventHandler;
class EventLoop : public SubSystem
{
  public:
    EventLoop(std::stop_token* stopToken);
    ~EventLoop();

    void registerListenerRawPtr(EventHandler* listener);
    void registerListener(std::unique_ptr<EventHandler> listener);
    // TODO: Might need to refactor this to use a shared_ptr or weak_ptr
    void deregisterListener(EventHandler* listener);
    int getListenersCount() const
    {
        return listeners.size();
    }

    void submitEvents(const Event& event);

  private:
    // SubSystem methods
    void init() override;
    void run();
    void shutdown() override;
    void handleTickEvent(std::chrono::steady_clock::time_point& lastTime);
    void handleInputEvents();

    std::list<std::unique_ptr<EventHandler>> listeners;
    std::list<EventHandler*> listenersRawPtrs;
    std::thread eventLoopThread;
    std::queue<Event> eventQueue;
};

} // namespace aion

#endif // EVENTLOOP_H