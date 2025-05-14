#ifndef EVENTPUBLISHER_H
#define EVENTPUBLISHER_H

#include "Event.h"

namespace aion
{
class EventPublisher
{
  public:
    virtual void publish(const Event& event) = 0;
};

} // namespace aion

#endif