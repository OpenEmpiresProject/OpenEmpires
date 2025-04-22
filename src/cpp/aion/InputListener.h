#ifndef INPUTLISTENER_H
#define INPUTLISTENER_H

#include "EventHandler.h"
#include "ThreadQueue.h"

namespace aion
{
class InputListener : public EventHandler
{
  public:
    InputListener(ThreadQueue& eventQueue) : eventQueue(eventQueue) {};
    virtual ~InputListener() = default;

  private:
    void onInit(EventLoop* eventLoop) override;
    void onExit() override;

    void onEvent(const Event& e) override;
    void handleInputEvents();

    ThreadQueue& eventQueue;
    EventLoop* eventLoop = nullptr;
};
} // namespace aion

#endif