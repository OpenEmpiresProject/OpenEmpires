#ifndef TILE_H
#define TILE_H

#include "Vec2Base.h"

namespace ion
{
struct TileTag
{
};
class Feet;
class Tile : public Vec2Base<int, TileTag>
{
  public:
    using Vec2Base<int, TileTag>::Vec2Base;

    Tile(const Vec2Base<int, TileTag>& base) : Vec2Base<int, TileTag>(base)
    {
    }
    Tile(const Feet& feet) = delete;

    Feet toFeet() const;
    Feet centerInFeet() const;
};
} // namespace ion

// Stream output
inline std::ostream& operator<<(std::ostream& os, const ion::Tile& v)
{
    return os << '(' << v.x << ", " << v.y << ')';
}

// Hash support
namespace std
{
template <> struct hash<ion::Tile>
{
    std::size_t operator()(const ion::Tile& v) const noexcept
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
template <std::size_t I> constexpr int get(const Tile& v) noexcept
{
    if constexpr (I == 0)
        return v.x;
    else if constexpr (I == 1)
        return v.y;
}
} // namespace ion

namespace std
{
template <> struct tuple_size<ion::Tile> : std::integral_constant<std::size_t, 2>
{
};

template <std::size_t I> struct tuple_element<I, ion::Tile>
{
    using type = int;
};
} // namespace std

#endif