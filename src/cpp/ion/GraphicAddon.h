#ifndef GRAPHICADDON_H
#define GRAPHICADDON_H

#include "Vec2d.h"

#include <cstdint>
#include <variant>

namespace ion
{
struct GraphicAddon
{
    enum class Type : uint8_t
    {
        NONE = 0,
        CIRCLE,
        SQUARE,
        RHOMBUS
    };

    struct Circle
    {
        int radius = 0;
        Vec2d center = {0, 0}; // Relative to anchor. i.e. actual unit position
    };

    struct Square
    {
        int width = 0;
        int height = 0;
        Vec2d center = {0, 0};
    };

    struct Rhombus // Relative to anchor
    {
        size_t width = 0;
        size_t height = 0;
    };

    using Data = std::variant<std::monostate, Circle, Square, Rhombus>;

    Type type = Type::NONE;
    Data data = std::monostate{};

    template <typename T> T getData() const
    {
        return std::get<T>(data);
    }
};
} // namespace ion

#endif