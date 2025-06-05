#ifndef EVENT_H
#define EVENT_H

#include "UnitSelection.h"
#include "Vec2d.h"

#include <variant>

namespace ion
{
struct TickData
{
    int deltaTimeMs = 0;
};

struct KeyboardData
{
    int keyCode = 0; // SDL_Scancode
};

struct MouseMoveData
{
    Vec2d screenPos;
};

struct MouseClickData
{
    enum class Button
    {
        LEFT = 0,
        RIGHT,
        MIDDLE
    };
    Button button;
    Vec2d screenPosition;
};

struct UnitSelectionData
{
    UnitSelection selection;
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
        MOUSE_BTN_DOWN,
        MOUSE_BTN_UP,
        UNIT_SELECTION,
        MAX_TYPE_MARKER,
    };

    using Data = std::variant<std::monostate,
                              TickData,
                              MouseClickData,
                              KeyboardData,
                              MouseMoveData,
                              UnitSelectionData>;

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
} // namespace ion

#endif