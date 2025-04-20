#include "Event.h"

using namespace aion;

Event::Event(Event::Type type) : type(type) {}

Event::Type Event::getType() const { return type; }
