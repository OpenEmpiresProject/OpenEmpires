#ifndef EVENT_H
#define EVENT_H

#include "Feet.h"
#include "Player.h"
#include "Tile.h"
#include "UnitSelection.h"
#include "commands/Command.h"

#include <entt/entity/registry.hpp>
#include <variant>

namespace core
{
struct TickData
{
    int deltaTimeMs = 0;
    int currentTick = 0;
};

struct KeyboardData
{
    int keyCode = 0; // SDL_Scancode
};

struct MouseMoveData
{
    Vec2 screenPos;
};

struct MouseClickData
{
    enum class Button
    {
        LEFT = 0,
        RIGHT,
        MIDDLE
    };
    Button button;
    Vec2 screenPosition;
};

struct UnitSelectionData
{
    UnitSelection selection;
};

struct CommandRequestData
{
    Command* command = nullptr;
    uint32_t entity = entt::null;
};

struct UnitTileMovementData
{
    uint32_t unit;
    Tile tile;
    Feet positionFeet;
};

struct BuildingPlacementData
{
    Ref<Player> player;
    uint32_t entityType = 0;
    Feet pos = Feet::null;
    uint32_t entity = entt::null;
};

struct EntityDeleteData
{
    uint32_t entity = entt::null;
};

struct UnitCreationData
{
    Ref<Player> player;
    Feet position;
    uint32_t entityType = 0;
};

struct Event
{
    enum class Type
    {
        NONE = 0,
        TICK,
        KEY_UP,
        KEY_DOWN,
        MOUSE_MOVE,
        MOUSE_BTN_DOWN,
        MOUSE_BTN_UP,
        UNIT_REQUESTED, // Use UnitCreationData
        UNIT_SELECTION,
        UNIT_TILE_MOVEMENT,
        ENTITY_DELETE,
        BUILDING_REQUESTED,          // Use BuildingPlacementData
        BUILDING_PLACEMENT_STARTED,  // Use BuildingPlacementData
        BUILDING_PLACEMENT_FINISHED, // Use BuildingPlacementData
        COMMAND_REQUEST,
        MAX_TYPE_MARKER,
    };

    using Data = std::variant<std::monostate,
                              TickData,
                              MouseClickData,
                              KeyboardData,
                              MouseMoveData,
                              UnitSelectionData,
                              CommandRequestData,
                              UnitTileMovementData,
                              EntityDeleteData,
                              UnitCreationData,
                              BuildingPlacementData>;

    const Type type = Type::NONE;
    const Data data = std::monostate{};

    template <typename T> T getData() const
    {
        return std::get<T>(data);
    }

    Event(Type type, Data data = {}) : type(type), data(std::move(data))
    {
    }
    Event() = delete;
};
} // namespace core

#endif