#ifndef CMDATTACK_H
#define CMDATTACK_H

#include "Coordinates.h"
#include "Feet.h"
#include "Player.h"
#include "ServiceRegistry.h"
#include "Settings.h"
#include "StateManager.h"
#include "commands/CmdDropResource.h"
#include "commands/CmdMove.h"
#include "commands/Command.h"
#include "components/CompEntityInfo.h"
#include "components/CompPlayer.h"
#include "components/CompResource.h"
#include "components/CompResourceGatherer.h"
#include "components/CompSelectible.h"
#include "components/CompTransform.h"
#include "debug.h"
#include "utils/LazyServiceRef.h"
#include "utils/ObjectPool.h"

#include <algorithm>
#include <queue>
#include <unordered_set>

namespace core
{
class CmdGatherResource : public Command
{
  public:
    uint32_t target = entt::null;

    /**
     * @brief Finds the closest resource entity of the specified type within a given radius from a
     * starting tile.
     *
     * Performs a breadth-first search (BFS) starting from the provided tile, expanding outward up
     * to maxRadius tiles. Returns the entity ID of the closest resource found, or entt::null if
     * none is found within the radius.
     *
     * @param resourceType The type of resource to search for.
     * @param startTile The tile from which to start the search.
     * @param maxRadius The maximum search radius (in tiles).
     * @return uint32_t The entity ID of the closest resource, or entt::null if not found.
     */
    static uint32_t findClosestResource(uint8_t resourceType,
                                        const Tile& startTile,
                                        int maxRadius,
                                        Ref<StateManager> stateMan);

    /**
     * @brief Checks if the specified tile contains a resource of the given type.
     *
     * This function examines the tile at the provided position to determine if it contains
     * a resource matching the specified resource type. If such a resource is found,
     * the resource is returned via argument and the function returns true.
     *
     * @param resourceType The type of resource to check for.
     * @param posTile The tile position to inspect.
     * @param resourceEntity Output parameter that receives the entity ID of the resource if found;
     * set to entt::null otherwise.
     * @return true if the tile contains a resource of the specified type; false otherwise.
     */
    static bool doesTileHaveResource(uint8_t resourceType,
                                     const Tile& posTile,
                                     uint32_t& resourceEntity,
                                     Ref<StateManager> stateMan);

  private:
    // TODO: This doesn't belong here, this should be in the components
    const int m_choppingSpeed = 10; // 10 wood per second
    // Cached values
    Feet m_targetPosition; // Use when target is absent by the time this command execute
    float m_collectedResourceAmount = 0;
    LazyServiceRef<Coordinates> m_coordinateSystem;
    LazyServiceRef<Settings> m_settings;
    uint8_t m_targetResourceType = 0;
    CompResourceGatherer* m_gatherer = nullptr;

  private:
    void onStart() override;
    void onQueue() override;
    std::string toString() const override;
    void destroy() override;

    /**
     * @brief Executes the gather resource command logic.
     *
     * This method checks if the resource is available and attempts to find another similar resource
     * if not. If the unit is close enough to the resource, it animates and gathers the resource for
     * the given time delta. Otherwise, it moves closer to the resource. If the unit's inventory is
     * full, it initiates a drop-off action.
     *
     * @param deltaTimeMs The elapsed time in milliseconds since the last execution.
     * @param subCommands A list to which any sub-commands (such as movement or drop-off actions)
     * may be added.
     * @return true if the command is complete and should be removed from the queue, false
     * otherwise.
     */
    bool onExecute(int deltaTimeMs, int currentTick, std::list<Command*>& subCommands) override;

    /**
     * @brief Gathers resources over time and updates the unit's collected amount.
     *
     * This function calculates the amount of resource gathered based on the gathering speed and
     * elapsed time. It updates the target resource and unit's storage, and marks the target entity
     * as dirty if any resource was gathered.
     *
     * @param deltaTimeMs The elapsed time in milliseconds since the last gather operation.
     */
    void gather(int deltaTimeMs);

    /**
     * @brief Attempts to find another resource of the same type near the unit.
     *
     * This function searches for the closest resource of the same type as the current target
     * resource, within a predefined maximum lookup radius. If a suitable resource is found, it sets
     * it as the new target.
     *
     * @return true if another similar resource was found and set as the new target; false
     * otherwise.
     */
    bool lookForAnotherSimilarResource();

    void moveCloser(std::list<Command*>& subCommands);
    void dropOff(std::list<Command*>& subCommands);
    bool isResourceAvailable() const;
    bool isCloseEnough() const;
    void animate(int currentTick);
    void setTarget(uint32_t targetEntity);
    bool isFull() const;
};

} // namespace core

#endif