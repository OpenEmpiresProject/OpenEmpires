#include "Event.h"

using namespace aion;

Event::Event(Event::Type type) : type(type)
{
}

Event::Event(Event::Type type, void* data) : type(type), data(data)
{
}

Event::Type Event::getType() const
{
    return type;
}
