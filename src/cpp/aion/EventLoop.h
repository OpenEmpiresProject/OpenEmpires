#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include "Event.h"
#include "EventListener.h"
#include "SubSystem.h"

#include <list>
#include <memory>
#include <thread>

namespace aion
{
class EventLoop : public SubSystem
{
  public:
    EventLoop(std::stop_token *stopToken);
    ~EventLoop();

    void registerListener(std::unique_ptr<EventListener> listener);
    // TODO: Might need to refactor this to use a shared_ptr or weak_ptr
    void deregisterListener(EventListener *listener);
    int getListenersCount() const { return listeners.size(); }

  private:
    // SubSystem methods
    void init() override;
    void run();
    void shutdown() override;

    std::list<std::unique_ptr<EventListener>> listeners;
    std::thread eventLoopThread;
};

} // namespace aion

#endif // EVENTLOOP_H