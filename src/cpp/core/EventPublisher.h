#ifndef EVENTPUBLISHER_H
#define EVENTPUBLISHER_H

#include "Event.h"
#include "utils/Types.h"

namespace core
{
void publishEvent(const Event& event);
void publishEvent(const Event::Type& type, const Event::Data& data);

class EventPublisher : public std::enable_shared_from_this<EventPublisher>
{
  public:
    virtual void publish(const Event& event) = 0;

  protected:
    void registerPublisher();

  private:
    friend void publishEvent(const Event& event);
    friend void publishEvent(const Event::Type& type, const Event::Data& data);
    thread_local static Ref<EventPublisher> s_instance;
};

} // namespace core

#endif