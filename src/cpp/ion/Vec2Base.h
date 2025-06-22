#ifndef VEC2BASE_H
#define VEC2BASE_H

#include <compare>
#include <cstddef>
#include <format>
#include <functional>
#include <iostream>
#include <utility>

namespace ion
{
template <typename T, typename Tag> class Vec2Base
{
  public:
    T x{};
    T y{};

    // Constructors
    constexpr Vec2Base() noexcept : x(0), y(0)
    {
    }
    constexpr Vec2Base(T x, T y) noexcept : x(x), y(y)
    {
    }

    // Copy / move
    constexpr Vec2Base(const Vec2Base&) noexcept = default;
    constexpr Vec2Base(Vec2Base&&) noexcept = default;
    constexpr Vec2Base& operator=(const Vec2Base&) noexcept = default;
    constexpr Vec2Base& operator=(Vec2Base&&) noexcept = default;

    // Comparison operators
    constexpr auto operator<=>(const Vec2Base&) const = default;

    bool operator==(const Vec2Base& other) const
    {
        return x == other.x && y == other.y;
    }

    bool operator!=(const Vec2Base& other) const
    {
        return x != other.x || y != other.y;
    }

    // Arithmetic operators
    constexpr Vec2Base operator+(const Vec2Base& other) const noexcept
    {
        return {x + other.x, y + other.y};
    }

    constexpr Vec2Base operator+(T delta) const noexcept
    {
        return {x + delta, y + delta};
    }

    constexpr Vec2Base operator-(const Vec2Base& other) const noexcept
    {
        return Vec2Base(x - other.x, y - other.y);
    }

    constexpr Vec2Base operator-(T delta) const noexcept
    {
        return Vec2Base(x - delta, y - delta);
    }

    constexpr Vec2Base operator*(T scalar) const noexcept
    {
        return {x * scalar, y * scalar};
    }

    constexpr Vec2Base operator/(T scalar) const noexcept
    {
        return {x / scalar, y / scalar};
    }

    constexpr Vec2Base& operator+=(const Vec2Base& other) noexcept
    {
        x += other.x;
        y += other.y;
        return *this;
    }

    constexpr Vec2Base& operator+=(T delta) noexcept
    {
        x += delta;
        y += delta;
        return *this;
    }

    constexpr Vec2Base& operator-=(const Vec2Base& other) noexcept
    {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    constexpr Vec2Base& operator*=(T scalar) noexcept
    {
        x *= scalar;
        y *= scalar;
        return *this;
    }

    constexpr Vec2Base& operator/=(T scalar) noexcept
    {
        x /= scalar;
        y /= scalar;
        return *this;
    }

    // Negation
    constexpr Vec2Base operator-() const noexcept
    {
        return {-x, -y};
    }

    // Magnitude squared
    constexpr T lengthSquared() const noexcept
    {
        return x * x + y * y;
    }

    T distanceSquared(const Vec2Base& other) const noexcept
    {
        return std::powl(x - other.x, 2) + std::powl(y - other.y, 2);
    }

    // Dot product
    constexpr T dot(const Vec2Base& other) const noexcept
    {
        return x * other.x + y * other.y;
    }

    std::string toString() const
    {
        return std::format("({}, {})", x, y);
    }

    constexpr void limitTo(T xLimit, T yLimit) noexcept
    {
        if (x > xLimit)
            x = xLimit;
        if (y > yLimit)
            y = yLimit;
    }
};

} // namespace ion

#endif // VEC2D_HPP
