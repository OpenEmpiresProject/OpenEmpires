#ifndef EVENT_H
#define EVENT_H

namespace aion
{
class Event
{
  public:
    enum class Type
    {
        NONE = 0,
        TICK
    };

    Event(Event::Type type = Event::Type::NONE);
    Type geType() const;

  private:
    const Event::Type type;
};
} // namespace aion

#endif