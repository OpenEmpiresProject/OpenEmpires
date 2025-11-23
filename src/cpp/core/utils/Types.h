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

static UnitType getUnitType(uint32_t unitTypeInt)
{
    switch (unitTypeInt)
    {
    case (uint32_t) UnitType::VILLAGER:
        return UnitType::VILLAGER;
    case (uint32_t) UnitType::INFANTRY:
        return UnitType::INFANTRY;
    case (uint32_t) UnitType::ARCHER:
        return UnitType::ARCHER;
    case (uint32_t) UnitType::CAVALRY:
        return UnitType::CAVALRY;
    case (uint32_t) UnitType::SHIP:
        return UnitType::SHIP;
    case (uint32_t) UnitType::SIEGE:
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

enum class BuildingOrientation
{
    NO_ORIENTATION = 0,
    RIGHT_ANGLED, // Use for both gate and wall
    LEFT_ANGLED,  // Use for both gate and wall
    CORNER,       // Use for wall
    HORIZONTAL,   // Use for both gate and wall
    VERTICAL,     // Use for both gate and wall

    // Max orientations can have is 8 (i.e. value 7) since this goes in the direction field of
    // GraphicsId
};

static std::string buildingOrientationToString(const BuildingOrientation& orientation)
{
    switch (orientation)
    {
    case BuildingOrientation::NO_ORIENTATION:
        return "no-orientation";
    case BuildingOrientation::RIGHT_ANGLED:
        return "right-angled";
    case BuildingOrientation::LEFT_ANGLED:
        return "left-angled";
    case BuildingOrientation::CORNER:
        return "corner";
    case BuildingOrientation::HORIZONTAL:
        return "horizontal";
    case BuildingOrientation::VERTICAL:
        return "vertical";
    }
    return "";
}

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

} // namespace core

#endif