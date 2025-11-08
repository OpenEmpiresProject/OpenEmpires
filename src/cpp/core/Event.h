#ifndef EVENT_H
#define EVENT_H

#include "EntitySelection.h"
#include "Feet.h"
#include "Player.h"
#include "Tile.h"
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

struct EntitySelectionData
{
    enum class Type
    {
        UNIT,
        BUILDING,
        NATURAL_RESOURCE
    };
    Type type;
    EntitySelection selection;
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
    uint32_t placementId = 0;
    bool seriesConstructionsAllowed = false;
    BuildingOrientation orientation = BuildingOrientation::DEFAULT;

    bool operator==(const BuildingPlacementData& other) const
    {
        return placementId == other.placementId;
    }

    BuildingPlacementData()
    {
        placementId = g_currentPlacementId++;
    }

  private:
    inline static uint32_t g_currentPlacementId = 0;
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

struct UnitQueueData
{
    Ref<Player> player;
    uint32_t entityType = 0; // Unit type
    uint32_t building = entt::null;
};

struct UngarrisonData
{
    Ref<Player> player;
    uint32_t building = entt::null;
};

struct GarrisonData
{
    Ref<Player> player;
    bool inprogress = true; // false means ended
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
        UNIT_CREATION_FINISHED, // Use UnitCreationData
        UNIT_QUEUE_REQUEST,     // UnitQueueData
        ENTITY_SELECTION,
        UNIT_TILE_MOVEMENT,
        ENTITY_DELETE,
        BUILDING_PLACEMENT_STARTED, // Use BuildingPlacementData. Sent only by active player
        BUILDING_PLACEMENT_ENDED,   // Use BuildingPlacementData. Sent only by active player
        BUILDING_REQUESTED,         // Use BuildingPlacementData
        BUILDING_CREATED,           // Use BuildingPlacementData
        COMMAND_REQUEST,
        UNGARRISON_REQUEST,
        GARRISON_REQUEST,
        MAX_TYPE_MARKER,
    };

    using Data = std::variant<std::monostate,
                              TickData,
                              MouseClickData,
                              KeyboardData,
                              MouseMoveData,
                              EntitySelectionData,
                              CommandRequestData,
                              UnitTileMovementData,
                              EntityDeleteData,
                              UnitQueueData,
                              UnitCreationData,
                              UngarrisonData,
                              GarrisonData,
                              BuildingPlacementData>;

    const Type type = Type::NONE;
    const Data data = std::monostate{};

    template <typename T> const T& getData() const
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