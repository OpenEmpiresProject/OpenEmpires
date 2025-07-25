#ifndef CMDATTACK_H
#define CMDATTACK_H

#include "Coordinates.h"
#include "Feet.h"
#include "GameState.h"
#include "Player.h"
#include "ServiceRegistry.h"
#include "commands/CmdDropResource.h"
#include "commands/CmdMove.h"
#include "commands/Command.h"
#include "components/CompDirty.h"
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

namespace ion
{
class CmdGatherResource : public Command
{
  public:
    uint32_t target = entt::null;

  private:
    // TODO: This doesn't belong here, this should be in the components
    const int choppingSpeed = 10; // 10 wood per second
    // Cached values
    Feet targetPosition; // Use when target is absent by the time this command execute
    float collectedResourceAmount = 0;
    std::shared_ptr<Coordinates> coordinateSystem;
    Ref<Player> player;
    Resource* targetResource = nullptr;
    CompResourceGatherer* gatherer = nullptr;

    void onStart() override
    {
        spdlog::debug("Start gathering resource...");
    }

    void onQueue() override
    {
        // TODO: Reset frame to zero (since this is a new command)
        setTarget(target);

        auto& playerComp = m_gameState->getComponent<CompPlayer>(m_entityID);
        player = playerComp.player;
        coordinateSystem = ServiceRegistry::getInstance().getService<Coordinates>();
        collectedResourceAmount = 0;
        gatherer = &(m_gameState->getComponent<CompResourceGatherer>(m_entityID));
    }

    void setTarget(uint32_t targetEntity)
    {
        target = targetEntity;
        auto& transformTarget = m_gameState->getComponent<CompTransform>(target);
        targetPosition = transformTarget.position;

        debug_assert(m_gameState->hasComponent<CompResource>(target),
                     "Target entity is not a resource");

        targetResource = &(m_gameState->getComponent<CompResource>(target).resource);
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
    bool onExecute(int deltaTimeMs, std::list<Command*>& subCommands) override
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
            animate();
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
                      targetPosition.toString());
        auto move = ObjectPool<CmdMove>::acquire();
        move->targetEntity = target;
        move->setPriority(getPriority() + CHILD_PRIORITY_OFFSET);
        subCommands.push_back(move);
    }

    void dropOff(std::list<Command*>& subCommands)
    {
        spdlog::debug("Unit {} is at full capacity, need to drop off", m_entityID);
        auto drop = ObjectPool<CmdDropResource>::acquire();
        drop->resourceType = targetResource->type;
        drop->setPriority(getPriority() + CHILD_PRIORITY_OFFSET);
        subCommands.push_back(drop);
    }

    bool isResourceAvailable()
    {
        if (target != entt::null)
        {
            CompEntityInfo info = m_gameState->getComponent<CompEntityInfo>(target);
            return !info.isDestroyed;
        }
        return false;
    }

    bool isCloseEnough()
    {
        auto& transformMy = m_gameState->getComponent<CompTransform>(m_entityID);
        auto threshold = transformMy.goalRadius + Constants::FEET_PER_TILE / 2;
        auto distanceSquared = transformMy.position.distanceSquared(targetPosition);
        // spdlog::debug("Check closeness. my {}, target {}", transformMy.position.toString(),
        // targetPosition.toString()); spdlog::debug("Check closeness. distance {}, threshold {}",
        // distanceSquared, threshold * threshold);
        return transformMy.position.distanceSquared(targetPosition) < (threshold * threshold);
    }

    void animate()
    {
        auto [transform, action, animation, dirty] =
            m_gameState->getComponents<CompTransform, CompAction, CompAnimation, CompDirty>(
                m_entityID);

        action.action = gatherer->getGatheringAction(targetResource->type);
        auto& actionAnimation = animation.animations[action.action];

        auto ticksPerFrame = m_settings->getTicksPerSecond() / actionAnimation.speed;
        if (s_totalTicks % ticksPerFrame == 0)
        {
            dirty.markDirty(m_entityID);
            animation.frame++;
            animation.frame %= actionAnimation.frames;
        }
    }

    bool isFull()
    {
        return gatherer->gatheredAmount >= gatherer->capacity;
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
        auto& transform = m_gameState->getComponent<CompTransform>(m_entityID);

        collectedResourceAmount += float(choppingSpeed) * deltaTimeMs / 1000.0f;
        uint32_t rounded = collectedResourceAmount;
        collectedResourceAmount -= rounded;

        if (rounded != 0)
        {
            auto actualDelta = std::min(targetResource->amount, rounded);
            actualDelta = std::min(actualDelta, (gatherer->capacity - gatherer->gatheredAmount));
            targetResource->amount -= actualDelta;
            gatherer->gatheredAmount += actualDelta;
            m_gameState->getComponent<CompDirty>(target).markDirty(target);
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
        auto& transformMy = m_gameState->getComponent<CompTransform>(m_entityID);
        auto tilePos = transformMy.position.toTile();

        auto newResource = findClosestResource(targetResource->type, tilePos,
                                               Constants::MAX_RESOURCE_LOOKUP_RADIUS);

        if (newResource != entt::null)
        {
            spdlog::debug("Found resource {}", newResource);
            setTarget(newResource);
        }
        else
        {
            spdlog::debug("No resource of type {} found around {}", targetResource->type,
                          transformMy.position.toString());
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
    bool doesTileHaveResource(uint8_t resourceType, const Tile& posTile, uint32_t& resourceEntity)
    {
        resourceEntity = entt::null;
        auto& map = m_gameState->gameMap;
        auto e = map.getEntity(MapLayerType::STATIC, posTile);
        if (e != entt::null)
        {
            CompEntityInfo info = m_gameState->getComponent<CompEntityInfo>(e);
            if (!info.isDestroyed)
            {
                if (m_gameState->hasComponent<CompResource>(e))
                {
                    CompResource resource = m_gameState->getComponent<CompResource>(e);
                    if (resource.resource.type == resourceType)
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
    uint32_t findClosestResource(uint8_t resourceType, const Tile& startTile, int maxRadius)
    {
        std::queue<std::pair<Tile, int>> toVisit; // Pair: position, current distance
        std::unordered_set<Tile> visited;

        uint32_t resourceEntity = entt::null;

        toVisit.push({startTile, 0});
        visited.insert(startTile);

        const std::vector<Tile> directions = {{0, -1},  {1, 0},  {0, 1}, {-1, 0},
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

} // namespace ion

#endif