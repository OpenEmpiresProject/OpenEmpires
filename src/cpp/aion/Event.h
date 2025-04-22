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
        TICK,
        KEY_UP,
        KEY_DOWN,
        MOUSE_MOVE,
        MOUSE_BTN_DOWN,
        MOUSE_BTN_UP,
        MAX_TYPE_MARKER
    };

    Event(Event::Type type = Event::Type::NONE);
    Event(Event::Type type, void* data);
    Type getType() const;
    template <typename T> T getData() const
    {
        return static_cast<T>(data);
    }

  private:
    const Event::Type type;
    void* data;
};
} // namespace aion

#endif