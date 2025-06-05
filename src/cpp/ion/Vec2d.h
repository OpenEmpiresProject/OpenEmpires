#ifndef VEC2D_H
#define VEC2D_H

#include <compare>
#include <cstddef>
#include <format>
#include <functional>
#include <iostream>
#include <utility>

namespace ion
{
class Vec2d
{
  public:
    int x;
    int y;

    // Constructors
    constexpr Vec2d() noexcept : x(0), y(0)
    {
    }
    constexpr Vec2d(int x, int y) noexcept : x(x), y(y)
    {
    }

    // Copy / move
    constexpr Vec2d(const Vec2d&) noexcept = default;
    constexpr Vec2d(Vec2d&&) noexcept = default;
    constexpr Vec2d& operator=(const Vec2d&) noexcept = default;
    constexpr Vec2d& operator=(Vec2d&&) noexcept = default;

    // Comparison operators
    constexpr auto operator<=>(const Vec2d&) const = default;

    bool operator==(const Vec2d& other) const
    {
        return x == other.x && y == other.y;
    }

    bool operator!=(const Vec2d& other) const
    {
        return x != other.x || y != other.y;
    }

    // Arithmetic operators
    constexpr Vec2d operator+(const Vec2d& other) const noexcept
    {
        return {x + other.x, y + other.y};
    }

    constexpr Vec2d operator-(const Vec2d& other) const noexcept
    {
        return Vec2d(x - other.x, y - other.y);
    }

    constexpr Vec2d operator*(int scalar) const noexcept
    {
        return {x * scalar, y * scalar};
    }

    constexpr Vec2d operator/(int scalar) const noexcept
    {
        return {x / scalar, y / scalar};
    }

    constexpr Vec2d& operator+=(const Vec2d& other) noexcept
    {
        x += other.x;
        y += other.y;
        return *this;
    }

    constexpr Vec2d& operator-=(const Vec2d& other) noexcept
    {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    constexpr Vec2d& operator*=(int scalar) noexcept
    {
        x *= scalar;
        y *= scalar;
        return *this;
    }

    constexpr Vec2d& operator/=(int scalar) noexcept
    {
        x /= scalar;
        y /= scalar;
        return *this;
    }

    // Negation
    constexpr Vec2d operator-() const noexcept
    {
        return {-x, -y};
    }

    // Magnitude squared
    constexpr int lengthSquared() const noexcept
    {
        return x * x + y * y;
    }

    int distanceSquared(const Vec2d& other) const noexcept
    {
        return std::powl(x - other.x, 2) + std::powl(y - other.y, 2);
    }

    // Dot product
    constexpr int dot(const Vec2d& other) const noexcept
    {
        return x * other.x + y * other.y;
    }

    std::string toString() const
    {
        return std::format("({}, {})", x, y);
    }
};

} // namespace ion

// Stream output
inline std::ostream& operator<<(std::ostream& os, const ion::Vec2d& v)
{
    return os << '(' << v.x << ", " << v.y << ')';
}

// Hash support
namespace std
{
template <> struct hash<ion::Vec2d>
{
    std::size_t operator()(const ion::Vec2d& v) const noexcept
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
template <std::size_t I> constexpr int get(const Vec2d& v) noexcept
{
    if constexpr (I == 0)
        return v.x;
    else if constexpr (I == 1)
        return v.y;
}
} // namespace ion

namespace std
{
template <> struct tuple_size<ion::Vec2d> : std::integral_constant<std::size_t, 2>
{
};

template <std::size_t I> struct tuple_element<I, ion::Vec2d>
{
    using type = int;
};
} // namespace std

#endif // VEC2D_HPP
