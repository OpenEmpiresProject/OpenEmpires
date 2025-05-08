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

    void registerListener(std::shared_ptr<EventHandler> listener);
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

    std::list<std::shared_ptr<EventHandler>> listeners;
    std::thread eventLoopThread;
    std::queue<Event> eventQueue;

    bool* previousKeyboardState = nullptr;
    uint32_t previousMouseState = 0;
    int previouseMouseX = 0;
    int previouseMouseY = 0;
};

} // namespace aion

#endif // EVENTLOOP_H