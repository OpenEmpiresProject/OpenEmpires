#ifndef EVENTLOOPLISTENER_H
#define EVENTLOOPLISTENER_H

#include "Event.h"

namespace aion
{
class EventLoopListener
{
  public:
    virtual void onInit() = 0;
    virtual void onExit() = 0;
    virtual void onEvent(const Event &e) = 0;
};
} // namespace aion

#endif