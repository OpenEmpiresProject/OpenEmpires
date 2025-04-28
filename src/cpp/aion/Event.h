#ifndef EVENT_H
#define EVENT_H

#include "Vec2d.h"

#include <variant>

namespace aion
{
struct MouseClickData
{
    enum class Button
    {
        LEFT = 0,
        RIGHT,
        MIDDLE
    };
    Button button;
    Vec2d feetPosition;
};

struct Event
{
    enum class Type
    {
        NONE = 0,
        TICK,
        KEY_UP,
        KEY_DOWN,
        MOUSE_MOVE,
        MOUSE_BTN_UP,
        MAX_TYPE_MARKER
    };

    using Data = std::variant<std::monostate, MouseClickData>;

    const Type type = Type::NONE;
    const Data data = std::monostate{};

    template <typename T> T getData() const
    {
        return std::get<T>(data);
    }

    Event(Type type, Data data = {}) : type(type), data(std::move(data))
    {
    }
    Event() = delete;
};
} // namespace aion

#endif