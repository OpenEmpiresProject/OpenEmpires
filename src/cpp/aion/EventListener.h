#ifndef EVENTLISTENER_H
#define EVENTLISTENER_H

#include "Event.h"

namespace aion
{
class EventListener
{
  public:
    virtual void onInit() = 0;
    virtual void onExit() = 0;
    virtual void onEvent(const Event &e) = 0;
};
} // namespace aion

#endif