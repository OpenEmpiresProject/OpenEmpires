#ifndef CORE_UTILS_H
#define CORE_UTILS_H
#include "Tile.h"
#include "Types.h"

#include <variant>

namespace core
{
class Utils
{
  public:
    Utils() = delete;

    static void calculateConnectedBuildingsPath(
        const Tile& start, const Tile& end, std::list<TilePosWithOrientation>& connectedBuildings);

    template <typename T, typename... Ts>
    static T get_or(const std::variant<Ts...>& v, T default_value)
    {
        if (const T* ptr = std::get_if<T>(&v))
            return *ptr;
        return default_value;
    }
};
} // namespace core

#endif // CORE_UTILS_H
