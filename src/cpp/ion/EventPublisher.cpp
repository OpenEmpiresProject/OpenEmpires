#include "EventPublisher.h"

namespace ion
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

void EventPublisher::registerPublisher()
{
    s_instance = shared_from_this();
}
} // namespace ion
