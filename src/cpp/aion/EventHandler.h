#ifndef EVENTHANDLER_H
#define EVENTHANDLER_H

#include "Event.h"
#include "EventLoop.h"

#include <queue>

namespace aion
{
class EventHandler
{
  public:
    virtual void onInit(EventLoop* eventLoop) = 0;
    virtual void onExit() = 0;
    virtual void onEvent(const Event& e) = 0;
};
} // namespace aion

#endif