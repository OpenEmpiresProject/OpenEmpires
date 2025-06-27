#ifndef FEET_H
#define FEET_H

#include "Vec2Base.h"
#include "utils/Types.h"

namespace ion
{
struct FeetTag
{
};
class Tile;
class Feet : public Vec2Base<float, FeetTag>
{
  public:
    using Vec2Base<float, FeetTag>::Vec2Base;

    Feet(const Vec2Base<float, FeetTag>& base) : Vec2Base<float, FeetTag>(base)
    {
    }
    Feet(const Tile& tile) = delete;

    bool isNull() const
    {
        return std::isnan(x) || std::isnan(y);
    }

    inline Feet normalized10() const
    {
        int lenSq = x * x + y * y;
        if (lenSq == 0)
            return {0, 0};

        float len = std::sqrt(static_cast<float>(lenSq));
        float scale = 10.0f / len;
        return {std::round(x * scale), std::round(y * scale)};
    }

    Tile toTile() const;
    Vec2 toVec2() const;

    static const Feet null;
};
} // namespace ion

// Stream output
inline std::ostream& operator<<(std::ostream& os, const ion::Feet& v)
{
    return os << '(' << v.x << ", " << v.y << ')';
}

// Hash support
namespace std
{
template <> struct hash<ion::Feet>
{
    std::size_t operator()(const ion::Feet& v) const noexcept
    {
        std::size_t h1 = std::hash<int>{}(v.x);
        std::size_t h2 = std::hash<int>{}(v.y);
        return h1 ^ (h2 << 1); // Combine hashes
    }
};
} // namespace std

// Structured bindings support
namespace ion
{
template <std::size_t I> constexpr int get(const Feet& v) noexcept
{
    if constexpr (I == 0)
        return v.x;
    else if constexpr (I == 1)
        return v.y;
}
} // namespace ion

namespace std
{
template <> struct tuple_size<ion::Feet> : std::integral_constant<std::size_t, 2>
{
};

template <std::size_t I> struct tuple_element<I, ion::Feet>
{
    using type = int;
};
} // namespace std

#endif