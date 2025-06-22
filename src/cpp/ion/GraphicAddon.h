#ifndef GRAPHICADDON_H
#define GRAPHICADDON_H

#include "Color.h"
#include "Feet.h"
#include "utils/Types.h"

#include <cstdint>
#include <variant>

namespace ion
{
struct GraphicAddon
{
    enum class Type : uint8_t
    {
        NONE = 0,
        ISO_CIRCLE,
        SQUARE,
        RHOMBUS,
        TEXT
    };

    struct IsoCircle
    {
        int radius = 0;
        Vec2 center; // Relative to anchor. i.e. actual unit position
    };

    struct Square
    {
        int width = 0;
        int height = 0;
        Vec2 center;
    };

    struct Rhombus // Relative to anchor
    {
        size_t width = 0;
        size_t height = 0;
    };

    struct Text
    {
        std::string text;
        Color color;
    };

    using Data = std::variant<std::monostate, IsoCircle, Square, Rhombus, Text>;

    Type type = Type::NONE;
    Data data = std::monostate{};

    template <typename T> T getData() const
    {
        return std::get<T>(data);
    }
};
} // namespace ion

#endif