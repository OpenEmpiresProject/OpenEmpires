#ifndef CMDATTACK_H
#define CMDATTACK_H

#include "Coordinates.h"
#include "Feet.h"
#include "Player.h"
#include "ServiceRegistry.h"
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

  private:
    // TODO: This doesn't belong here, this should be in the components
    const int m_choppingSpeed = 10; // 10 wood per second
    // Cached values
    Feet m_targetPosition; // Use when target is absent by the time this command execute
    float m_collectedResourceAmount = 0;
    std::shared_ptr<Coordinates> m_coordinateSystem;
    uint8_t m_targetResourceType = 0;
    CompResourceGatherer* m_gatherer = nullptr;

  private:
    void onStart() override
    {
        spdlog::debug("Start gathering resource...");
    }

    void onQueue() override
    {
        // TODO: Reset frame to zero (since this is a new command)
        setTarget(target);

        m_coordinateSystem = ServiceRegistry::getInstance().getService<Coordinates>();
        m_collectedResourceAmount = 0;
        m_gatherer = &(m_stateMan->getComponent<CompResourceGatherer>(m_entityID));
    }

    void setTarget(uint32_t targetEntity)
    {
        debug_assert(m_stateMan->hasComponent<CompResource>(targetEntity),
                     "Target entity is not a resource");

        target = targetEntity;

        auto [transformTarget, resourceTarget] =
            m_stateMan->getComponents<CompTransform, CompResource>(target);
        m_targetPosition = transformTarget.position;
        m_targetResourceType = resourceTarget.original.value().type;
    }

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
    bool onExecute(int deltaTimeMs, int currentTick, std::list<Command*>& subCommands) override
    {
        if (isResourceAvailable() == false)
        {
            if (lookForAnotherSimilarResource() == false)
            {
                // Could not find another resource, nothing to gather, done with the command
                return true;
            }
        }

        if (isCloseEnough())
        {
            animate(currentTick);
            gather(deltaTimeMs);
        }
        else
        {
            moveCloser(subCommands);
        }

        if (isFull())
        {
            dropOff(subCommands);
        }
        return false;
    }

    std::string toString() const override
    {
        return "gather-resource";
    }

    void destroy() override
    {
        ObjectPool<CmdGatherResource>::release(this);
    }

    void moveCloser(std::list<Command*>& subCommands)
    {
        spdlog::debug("Target {} at {} is not close enough to gather, moving...", target,
                      m_targetPosition.toString());
        auto moveCmd = ObjectPool<CmdMove>::acquire();
        moveCmd->targetEntity = target;
        moveCmd->setPriority(getPriority() + CHILD_PRIORITY_OFFSET);
        subCommands.push_back(moveCmd);
    }

    void dropOff(std::list<Command*>& subCommands)
    {
        spdlog::debug("Unit {} is at full capacity, need to drop off", m_entityID);
        auto dropCmd = ObjectPool<CmdDropResource>::acquire();
        dropCmd->resourceType = m_targetResourceType;
        dropCmd->setPriority(getPriority() + CHILD_PRIORITY_OFFSET);
        subCommands.push_back(dropCmd);
    }

    bool isResourceAvailable() const
    {
        if (target != entt::null)
        {
            const auto& info = m_stateMan->getComponent<CompEntityInfo>(target);
            return !info.isDestroyed;
        }
        return false;
    }

    bool isCloseEnough() const
    {
        const auto& transformMy = m_components->transform;
        auto threshold = transformMy.goalRadius + Constants::FEET_PER_TILE / 2;
        auto distanceSquared = transformMy.position.distanceSquared(m_targetPosition);
        return transformMy.position.distanceSquared(m_targetPosition) < (threshold * threshold);
    }

    void animate(int currentTick)
    {
        m_components->action.action = m_gatherer->getGatheringAction(m_targetResourceType);
        auto& actionAnimation = m_components->animation.animations[m_components->action.action];

        auto ticksPerFrame = m_settings->getTicksPerSecond() / actionAnimation.value().speed;
        if (currentTick % ticksPerFrame == 0)
        {
            StateManager::markDirty(m_entityID);
            m_components->animation.frame++;
            m_components->animation.frame %= actionAnimation.value().frames;
        }
    }

    bool isFull() const
    {
        return m_gatherer->gatheredAmount >= m_gatherer->capacity;
    }

    /**
     * @brief Gathers resources over time and updates the unit's collected amount.
     *
     * This function calculates the amount of resource gathered based on the gathering speed and
     * elapsed time. It updates the target resource and unit's storage, and marks the target entity
     * as dirty if any resource was gathered.
     *
     * @param deltaTimeMs The elapsed time in milliseconds since the last gather operation.
     */
    void gather(int deltaTimeMs)
    {
        auto& resource = m_stateMan->getComponent<CompResource>(target);

        m_collectedResourceAmount += float(m_choppingSpeed) * deltaTimeMs / 1000.0f;
        uint32_t rounded = m_collectedResourceAmount;
        m_collectedResourceAmount -= rounded;

        if (rounded != 0)
        {
            auto actualDelta = std::min(resource.remainingAmount, rounded);
            actualDelta =
                std::min(actualDelta, (m_gatherer->capacity - m_gatherer->gatheredAmount));
            resource.remainingAmount -= actualDelta;
            m_gatherer->gatheredAmount += actualDelta;
            StateManager::markDirty(target);
        }
    }

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
    bool lookForAnotherSimilarResource()
    {
        spdlog::debug("Looking for another resource...");
        auto tilePos = m_components->transform.position.toTile();

        auto newResource = findClosestResource(m_targetResourceType, tilePos,
                                               Constants::MAX_RESOURCE_LOOKUP_RADIUS);

        if (newResource != entt::null)
        {
            spdlog::debug("Found resource {}", newResource);
            setTarget(newResource);
        }
        else
        {
            spdlog::debug("No resource of type {} found around {}", m_targetResourceType,
                          m_components->transform.position.toString());
        }
        return newResource != entt::null;
    }

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
    bool doesTileHaveResource(uint8_t resourceType,
                              const Tile& posTile,
                              uint32_t& resourceEntity) const
    {
        resourceEntity = entt::null;
        auto& map = m_stateMan->gameMap();
        auto e = map.getEntity(MapLayerType::STATIC, posTile);
        if (e != entt::null)
        {
            const auto& info = m_stateMan->getComponent<CompEntityInfo>(e);
            if (!info.isDestroyed)
            {
                if (m_stateMan->hasComponent<CompResource>(e))
                {
                    const auto& resource = m_stateMan->getComponent<CompResource>(e);
                    if (resource.original.value().type == resourceType)
                    {
                        resourceEntity = e;
                        return true;
                    }
                }
            }
        }
        return false;
    }

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
    uint32_t findClosestResource(uint8_t resourceType, const Tile& startTile, int maxRadius) const
    {
        std::queue<std::pair<Tile, int>> toVisit; // Pair: position, current distance
        std::unordered_set<Tile> visited;

        uint32_t resourceEntity = entt::null;

        toVisit.push({startTile, 0});
        visited.insert(startTile);

        static const std::vector<Tile> directions = {{0, -1},  {1, 0},  {0, 1}, {-1, 0},
                                                     {-1, -1}, {1, -1}, {1, 1}, {-1, 1}};

        while (!toVisit.empty())
        {
            auto [current, distance] = toVisit.front();
            toVisit.pop();

            if (doesTileHaveResource(resourceType, current, resourceEntity))
            {
                return resourceEntity;
            }

            if (distance >= maxRadius)
            {
                continue; // Stop expanding beyond max radius
            }

            for (const auto& dir : directions)
            {
                Tile neighbor{current.x + dir.x, current.y + dir.y};

                if (!visited.contains(neighbor))
                {
                    visited.insert(neighbor);
                    toVisit.push({neighbor, distance + 1});
                }
            }
        }
        return entt::null; // Not found within radius
    }
};

} // namespace core

#endif