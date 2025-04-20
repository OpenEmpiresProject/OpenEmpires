#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include "Event.h"
#include "EventLoopListener.h"
#include "SubSystem.h"

#include <list>
#include <memory>
#include <thread>

namespace aion
{
class EventLoop : public SubSystem
{
  public:
    EventLoop();
    ~EventLoop();

    void registerListener(std::unique_ptr<EventLoopListener> listener);
    // TODO: Might need to refactor this to use a shared_ptr or weak_ptr
    void deregisterListener(EventLoopListener *listener);
    int getListenersCount() const { return listeners.size(); }

  private:
    // SubSystem methods
    void init() override;
    void run();
    void shutdown() override;

    std::list<std::unique_ptr<EventLoopListener>> listeners;
    std::thread eventLoopThread;
};

} // namespace aion

#endif // EVENTLOOP_H