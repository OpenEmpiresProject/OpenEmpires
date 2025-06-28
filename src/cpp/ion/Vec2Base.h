#ifndef VEC2BASE_H
#define VEC2BASE_H

#include <cmath>
#include <compare>
#include <cstddef>
#include <format>
#include <functional>
#include <iostream>
#include <limits>
#include <type_traits>
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

    const static Vec2Base null;
    const static Vec2Base zero;

    constexpr bool isNull() const
    {
        return std::isnan(x) || std::isnan(y);
    }

    // Epsilon for floating point comparisons
    static constexpr T epsilon = static_cast<T>(1e-2);

    constexpr bool operator==(const Vec2Base& other) const noexcept
    {
        if constexpr (std::is_floating_point_v<T>)
        {
            return (std::abs(x - other.x) < epsilon) && (std::abs(y - other.y) < epsilon);
        }
        else
        {
            return x == other.x && y == other.y;
        }
    }

    constexpr bool operator!=(const Vec2Base& other) const noexcept
    {
        return !(*this == other);
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

    constexpr bool operator<(const Vec2Base& other) const noexcept
    {
        return x < other.x && y < other.y;
    }

    constexpr bool operator>(const Vec2Base& other) const noexcept
    {
        return x > other.x && y > other.y;
    }

    // Magnitude squared
    constexpr T lengthSquared() const noexcept
    {
        return x * x + y * y;
    }

    constexpr T length() const noexcept
    {
        return std::sqrt(x * x + y * y);
    }

    T distanceSquared(const Vec2Base& other) const noexcept
    {
        return std::pow(x - other.x, 2) + std::pow(y - other.y, 2);
    }

    T distance(const Vec2Base& other) const noexcept
    {
        return std::sqrt(distanceSquared(other));
    }

    constexpr Vec2Base normalized() const noexcept
    {
        T len = length();
        if (len == T(0))
            return Vec2Base(0, 0);
        return Vec2Base(x / len, y / len);
    }

    // Dot product
    constexpr T dot(const Vec2Base& other) const noexcept
    {
        return x * other.x + y * other.y;
    }

    std::string toString() const
    {
        if constexpr (std::is_floating_point_v<T>)
        {
            return std::format("({:.2f}, {:.2f})", x, y);
        }
        else
        {
            return std::format("({}, {})", x, y);
        }
    }

    constexpr void limitTo(T xLimit, T yLimit) noexcept
    {
        if (x > xLimit)
            x = xLimit;
        if (y > yLimit)
            y = yLimit;
    }
};

template <typename T, typename Tag> const Vec2Base<T, Tag> Vec2Base<T, Tag>::zero = Vec2Base(0, 0);

template <typename T, typename Tag>
const Vec2Base<T, Tag> Vec2Base<T, Tag>::null =
    Vec2Base(std::numeric_limits<T>::quiet_NaN(), std::numeric_limits<T>::quiet_NaN());

} // namespace ion

#endif // VEC2D_HPP
