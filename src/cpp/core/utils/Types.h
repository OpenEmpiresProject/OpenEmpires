#ifndef TYPES_H
#define TYPES_H

#include "Tile.h"
#include "Vec2Base.h"
#include "utils/Constants.h"

#include <bitset>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#define DEVELOPMENT

namespace core
{
template <typename T> using Ref = std::shared_ptr<T>;

template <typename T, typename... Args> Ref<T> CreateRef(Args&&... args)
{
    return std::make_shared<T>(std::forward<Args>(args)...);
}

using Signature = std::bitset<Constants::MAX_COMPONENTS>;

struct GenericTag
{
};
using Vec2 = Vec2Base<float, GenericTag>;

enum class WorldSizeType
{
    TEST,
    DEMO,
    TINY,
    MEDIUM,
    GIANT
};

enum Direction
{
    NORTH = 0,
    NORTHEAST,
    EAST,
    SOUTHEAST,
    SOUTH,
    SOUTHWEST,
    WEST,
    NORTHWEST,
    NONE
};

static std::string directionToString(Direction direction)
{
    switch (direction)
    {
    case Direction::NORTH:
        return "NORTH";
    case Direction::NORTHEAST:
        return "NORTHEAST";
    case Direction::EAST:
        return "EAST";
    case Direction::SOUTHEAST:
        return "SOUTHEAST";
    case Direction::SOUTH:
        return "SOUTH";
    case Direction::SOUTHWEST:
        return "SOUTHWEST";
    case Direction::WEST:
        return "WEST";
    case Direction::NORTHWEST:
        return "NORTHWEST";
    default:
        return "UNKNOWN";
    }
}

enum UnitAction
{
    IDLE = 0,
    MOVE = 1,
    CHOPPING = 2,
    MINING = 3,
    BUILDING = 4,
    CARRYING_LUMBER = 5,
    CARRYING_GOLD = 6,
    CARRYING_STONE = 7,
    ATTACK = 8,
    DIE = 9,
    DECAY_CORPSE = 10,
};

enum class UnitType
{
    UNKNOWN = 0,
    VILLAGER = 1,
    INFANTRY = 2,
    ARCHER = 3,
    CAVALRY = 4,
    SHIP = 5,
    SIEGE = 6
};

// static UnitType getUnitType(const std::string& value)
//{
//     if (value == "1")
//         return UnitType::VILLAGER;
//     else if (value == "2")
//         return UnitType::INFANTRY;
//     else if (value == "3")
//         return UnitType::ARCHER;
//     else if (value == "4")
//         return UnitType::CAVALRY;
//     else if (value == "5")
//         return UnitType::SHIP;
//     else if (value == "6")
//         return UnitType::SIEGE;
//     else
//         return UnitType::UNKNOWN;
// }

static UnitType getUnitType(int unitTypeInt)
{
    switch (unitTypeInt)
    {
    case (int) UnitType::VILLAGER:
        return UnitType::VILLAGER;
    case (int) UnitType::INFANTRY:
        return UnitType::INFANTRY;
    case (int) UnitType::ARCHER:
        return UnitType::ARCHER;
    case (int) UnitType::CAVALRY:
        return UnitType::CAVALRY;
    case (int) UnitType::SHIP:
        return UnitType::SHIP;
    case (int) UnitType::SIEGE:
        return UnitType::SIEGE;
    default:
        break;
    }
    return UnitType::UNKNOWN;
}

enum class CursorType
{
    DEFAULT_UI,
    DEFAULT_INGAME,
    LOADING,
    ASSIGN_TASK,
    BUILD,
    ATTACK,
    GARRISON,
    CHOP_WOOD,
    FLAG
};

/*
 *   Represent different visual representations of buildings depending on their facing
 *   direction.
 *   Note: Naming of the values are with respect to the screen, but not to logical game
 *   map. Eg: HORIZONTAL literally means horizontal in the screen, but logically diagonal.
 */
enum class BuildingOrientation
{
    NO_ORIENTATION = 0,

    /*
     *   Visual representation: ⟋
     *   Use: For both gate and wall
     */
    DIAGONAL_FORWARD,

    /*
     *   Visual representation: ⟍
     *   Use: For both gate and wall
     */
    DIAGONAL_BACKWARD,

    /*
     *   Use: For wall corners and starts/ends.
     */
    CORNER,

    /*
     *   Visual representation: ⎯
     *   Use: For both gate and wall
     */
    HORIZONTAL,

    /*
     *   Visual representation: |
     *   Use: For both gate and wall
     */
    VERTICAL,

    // Max orientations can have is 8 (i.e. value 7) since this goes in the direction field of
    // GraphicsId
};

static std::string buildingOrientationToString(const BuildingOrientation& orientation)
{
    switch (orientation)
    {
    case BuildingOrientation::NO_ORIENTATION:
        return "no-orientation";
    case BuildingOrientation::DIAGONAL_FORWARD:
        return "diagonal-forward";
    case BuildingOrientation::DIAGONAL_BACKWARD:
        return "diagonal-backward";
    case BuildingOrientation::CORNER:
        return "corner";
    case BuildingOrientation::HORIZONTAL:
        return "horizontal";
    case BuildingOrientation::VERTICAL:
        return "vertical";
    }
    return "";
}

struct TilePosWithOrientation
{
    Tile pos;
    BuildingOrientation orientation;
};

enum class UnitTagType
{
    GARRISONED
};

enum class RevealStatus : uint8_t
{
    NONE = 0,
    UNEXPLORED,
    EXPLORED,
    VISIBLE
};

enum class MapLayerType
{
    GROUND = 0,
    ON_GROUND, // walkable decorators/items on the ground
    STATIC,    // doesn't move, not walkable
    UNITS,
    // Add more layers here

    MAX_LAYERS
};

enum class LineOfSightShape
{
    CIRCLE = 0,
    ROUNDED_SQUARE,

    UNKNOWN
};

enum class Allegiance
{
    NEUTRAL = 0,
    ALLY,
    ENEMY
};

enum class Alignment : uint8_t
{
    TOP_LEFT,
    TOP_CENTER,
    TOP_RIGHT,
    CENTER_LEFT,
    CENTER,
    CENTER_RIGHT,
    BOTTOM_LEFT,
    BOTTOM_CENTER,
    BOTTOM_RIGHT
};

template <typename T> constexpr int toInt(const T& t) noexcept
{
    return static_cast<int>(t);
}

template <typename E, E Begin, E End> constexpr auto make_enum_array()
{
    constexpr auto count = static_cast<size_t>(End) - static_cast<size_t>(Begin);
    std::array<E, count> arr{};
    for (size_t i = 0; i < count; ++i)
        arr[i] = static_cast<E>(static_cast<size_t>(Begin) + i);
    return arr;
}

struct LandArea
{
    std::vector<Tile> tiles;
};

template <size_t N> struct FixedString
{
    char value[N]{};

    constexpr FixedString(const char (&str)[N])
    {
        std::copy_n(str, N, value);
    }

    // Needed for comparisons in constexpr contexts
    constexpr auto operator<=>(const FixedString&) const = default;
};

} // namespace core

#endif