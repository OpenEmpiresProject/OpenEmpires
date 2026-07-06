#include "EventPublisher.h"

namespace core
{
thread_local Ref<EventPublisher> EventPublisher::s_instance;

void publishEvent(const Event& event)
{
    EventPublisher::s_instance->publish(event);
}

void publishEvent(const Event::Type& type, const Event::Data& data)
{
    EventPublisher::s_instance->publish(Event{type, data});
}

void EventPublisher::registerPublisher(Ref<EventPublisher> publisher)
{
    s_instance = publisher;
}
} // namespace core
