#ifndef IONRECT_H
#define IONRECT_H

#include "Feet.h"

#include <concepts>
#include <iostream>
#include <optional>
#include <tuple>

namespace core
{
template <typename T> class Rect
{
  public:
    T x{};
    T y{};
    T w{};
    T h{};

    // Constructors
    constexpr Rect() = default;
    constexpr Rect(T x, T y, T w, T h) : x{x}, y{y}, w{w}, h{h}
    {
    }

    // Named constructors
    static constexpr Rect fromPositionAndSize(T x, T y, T w, T h)
    {
        return Rect(x, y, w, h);
    }

    static constexpr Rect fromTopLeftBottomRight(T x1, T y1, T x2, T y2)
    {
        return Rect(x1, y1, x2 - x1, y2 - y1);
    }

    // Utility
    constexpr Vec2 position() const
    {
        return Vec2(x, y);
    }

    constexpr T left() const
    {
        return x;
    }
    constexpr T right() const
    {
        return x + w;
    }
    constexpr T top() const
    {
        return y;
    }
    constexpr T bottom() const
    {
        return y + h;
    }

    constexpr T area() const
    {
        return w * h;
    }
    constexpr bool isEmpty() const
    {
        return w <= 0 || h <= 0;
    }

    constexpr bool contains(T px, T py) const
    {
        return px >= x && px < x + w && py >= y && py < y + h;
    }

    constexpr bool contains(const Vec2& pos) const
    {
        return pos.x >= x && pos.x < x + w && pos.y >= y && pos.y < y + h;
    }

    constexpr bool contains(const Rect& other) const
    {
        return contains(other.x, other.y) && contains(other.x + other.w - 1, other.y + other.h - 1);
    }

    constexpr bool intersects(const Rect& other) const
    {
        return !(other.right() <= left() || other.left() >= right() || other.bottom() <= top() ||
                 other.top() >= bottom());
    }

    constexpr std::optional<Rect> intersection(const Rect& other) const
    {
        T nx = std::max(x, other.x);
        T ny = std::max(y, other.y);
        T nr = std::min(right(), other.right());
        T nb = std::min(bottom(), other.bottom());
        if (nr > nx && nb > ny)
        {
            return Rect(nx, ny, nr - nx, nb - ny);
        }
        return std::nullopt;
    }

    constexpr Rect translated(const Rect<T>& rhs) const
    {
        return Rect(x + rhs.x, y + rhs.y, w, h);
    }

    constexpr Rect translated(T dx, T dy) const
    {
        return Rect(x + dx, y + dy, w, h);
    }

    constexpr Rect scaled(T sx, T sy) const
    {
        return Rect(x, y, w * sx, h * sy);
    }

    // Operators
    constexpr auto operator<=>(const Rect&) const = default;

    constexpr Rect operator+(const Rect& rhs) const
    {
        return Rect(x + rhs.x, y + rhs.y, w + rhs.w, h + rhs.h);
    }

    constexpr Rect operator-(const Rect& rhs) const
    {
        return Rect(x - rhs.x, y - rhs.y, w - rhs.w, h - rhs.h);
    }

    constexpr Rect& operator+=(const Rect& rhs)
    {
        x += rhs.x;
        y += rhs.y;
        w += rhs.w;
        h += rhs.h;
        return *this;
    }

    constexpr Rect& operator-=(const Rect& rhs)
    {
        x -= rhs.x;
        y -= rhs.y;
        w -= rhs.w;
        h -= rhs.h;
        return *this;
    }

    // Structured bindings support
    friend constexpr auto operator==(const Rect&, const Rect&) -> bool = default;

    friend std::ostream& operator<<(std::ostream& os, const Rect& r)
    {
        return os << "Rect(x=" << r.x << ", y=" << r.y << ", w=" << r.w << ", h=" << r.h << ")";
    }

    // Enable structured bindings
    template <std::size_t N> constexpr auto get() const
    {
        if constexpr (N == 0)
            return x;
        else if constexpr (N == 1)
            return y;
        else if constexpr (N == 2)
            return w;
        else if constexpr (N == 3)
            return h;
        else
            static_assert(N < 4, "Rect only has 4 elements");
    }

    std::string toString()
    {
        if constexpr (std::is_floating_point_v<T>)
        {
            return std::format("(x{:.2f}, y{:.2f}, w{:.2f}, h{:.2f})", x, y, w, h);
        }
        else
        {
            return std::format("(x{}, y{}, w{}, h{})", x, y, w, h);
        }
    }
};
} // namespace core

// Structured bindings support
namespace std
{
template <std::integral T> struct tuple_size<core::Rect<T>> : std::integral_constant<std::size_t, 4>
{
};

template <std::integral T> struct tuple_element<0, core::Rect<T>>
{
    using type = T;
};
template <std::integral T> struct tuple_element<1, core::Rect<T>>
{
    using type = T;
};
template <std::integral T> struct tuple_element<2, core::Rect<T>>
{
    using type = T;
};
template <std::integral T> struct tuple_element<3, core::Rect<T>>
{
    using type = T;
};
} // namespace std

#endif