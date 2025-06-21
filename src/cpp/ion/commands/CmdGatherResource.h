#ifndef CMDATTACK_H
#define CMDATTACK_H

#include "Coordinates.h"
#include "GameState.h"
#include "Player.h"
#include "ServiceRegistry.h"
#include "Vec2d.h"
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

namespace ion
{
class CmdGatherResource : public Command
{
  public:
    uint32_t target = entt::null;

  private:
    const int choppingSpeed = 10; // 10 wood per second
    // Cached values
    uint32_t entity = entt::null;
    Vec2d targetPosition; // Use when target is absent by the time this command execute
    float collectedResourceAmount = 0;
    std::shared_ptr<Coordinates> coordinateSystem;
    Ref<Player> player;
    Resource* targetResource = nullptr;

    void onStart() override
    {
        spdlog::debug("Start gathering resource...");
    }

    void onQueue(uint32_t entityID) override
    {
        // TODO: Reset frame to zero (since this is a new command)
        entity = entityID;
        setTarget(target);

        auto& playerComp = GameState::getInstance().getComponent<CompPlayer>(entity);
        player = playerComp.player;
        coordinateSystem = ServiceRegistry::getInstance().getService<Coordinates>();
        collectedResourceAmount = 0;
    }

    void setTarget(uint32_t targetEntity)
    {
        target = targetEntity;
        auto& transformTarget = GameState::getInstance().getComponent<CompTransform>(target);
        targetPosition = transformTarget.position;

        debug_assert(GameState::getInstance().hasComponent<CompResource>(target),
                     "Target entity is not a resource");

        targetResource = &GameState::getInstance().getComponent<CompResource>(target).resource;
    }

    bool onExecute(uint32_t entityID, int deltaTimeMs) override
    {
        if (!isResourceAvailable())
        {
            if (!lookForAnotherSimilarResource())
            {
                // Could not find another resource, nothing to gather, done with the command
                return true;
            }
        }
        if (isCloseEnough())
        {
            auto [transform, action, animation, dirty, gatherer] =
                GameState::getInstance()
                    .getComponents<CompTransform, CompAction, CompAnimation, CompDirty,
                                   CompResourceGatherer>(entityID);

            animate(action, gatherer, animation, dirty);
            gather(transform, deltaTimeMs);
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

    bool onCreateSubCommands(std::list<Command*>& subCommands) override
    {
        if (!isCloseEnough())
        {
            spdlog::debug("Target at {} is not close enough, moving...", targetPosition.toString());
            auto move = ObjectPool<CmdMove>::acquire();
            move->goal = targetPosition;
            move->setPriority(getPriority() + CHILD_PRIORITY_OFFSET);
            subCommands.push_back(move);
            return true;
        }
        return false;
    }

    bool isResourceAvailable()
    {
        if (target != entt::null)
        {
            CompEntityInfo info = GameState::getInstance().getComponent<CompEntityInfo>(target);
            return !info.isDestroyed;
        }
        return false;
    }

    bool isCloseEnough()
    {
        auto& transformMy = GameState::getInstance().getComponent<CompTransform>(entity);
        auto threshold = transformMy.goalRadius + Constants::FEET_PER_TILE / 2;
        auto distanceSquared = transformMy.position.distanceSquared(targetPosition);
        // spdlog::debug("Check closeness. my {}, target {}", transformMy.position.toString(),
        // targetPosition.toString()); spdlog::debug("Check closeness. distance {}, threshold {}",
        // distanceSquared, threshold * threshold);
        return transformMy.position.distanceSquared(targetPosition) < (threshold * threshold);
    }

    void animate(CompAction& action,
                 CompResourceGatherer& gatherer,
                 CompAnimation& animation,
                 CompDirty& dirty)
    {
        action.action = gatherer.getGatheringAction(targetResource->type);
        auto& actionAnimation = animation.animations[action.action];

        auto ticksPerFrame = m_settings->getTicksPerSecond() / actionAnimation.speed;
        if (s_totalTicks % ticksPerFrame == 0)
        {
            dirty.markDirty(entity);
            animation.frame++;
            animation.frame %= actionAnimation.frames;
        }
    }

    void gather(CompTransform& transform, int deltaTimeMs)
    {
        collectedResourceAmount += float(choppingSpeed) * deltaTimeMs / 1000.0f;
        uint32_t rounded = collectedResourceAmount;
        collectedResourceAmount -= rounded;

        if (rounded != 0)
        {
            auto& dirty = GameState::getInstance().getComponent<CompDirty>(target);

            auto actualDelta = std::min(targetResource->amount, rounded);
            targetResource->amount -= actualDelta;
            player->grantResource(targetResource->type, actualDelta);
            dirty.markDirty(target);
        }
    }

    bool lookForAnotherSimilarResource()
    {
        spdlog::debug("Looking for another resource...");
        auto& transformMy = GameState::getInstance().getComponent<CompTransform>(entity);
        auto tilePos = Coordinates::feetToTiles(transformMy.position);

        auto newResource = findClosestResource(targetResource->type, tilePos,
                                               Constants::MAX_RESOURCE_LOOKUP_RADIUS);

        if (newResource != entt::null)
        {
            spdlog::debug("Found resource {}", newResource);
            setTarget(newResource);
        }
        return newResource != entt::null;
    }

    bool hasResource(uint8_t resourceType, const Vec2d& posTile, uint32_t& resourceEntity)
    {
        resourceEntity = entt::null;
        auto& map = GameState::getInstance().gameMap;
        auto e = map.getEntity(MapLayerType::STATIC, posTile);
        if (e != entt::null)
        {
            CompEntityInfo info = GameState::getInstance().getComponent<CompEntityInfo>(e);
            if (!info.isDestroyed)
            {
                if (GameState::getInstance().hasComponent<CompResource>(e))
                {
                    CompResource resource = GameState::getInstance().getComponent<CompResource>(e);
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

    uint32_t findClosestResource(uint8_t resourceType, const Vec2d& startTile, int maxRadius)
    {
        std::queue<std::pair<Vec2d, int>> toVisit; // Pair: position, current distance
        std::unordered_set<Vec2d> visited;

        uint32_t resourceEntity = entt::null;

        toVisit.push({startTile, 0});
        visited.insert(startTile);

        const std::vector<Vec2d> directions = {{0, -1},  {1, 0},  {0, 1}, {-1, 0},
                                               {-1, -1}, {1, -1}, {1, 1}, {-1, 1}};

        while (!toVisit.empty())
        {
            auto [current, distance] = toVisit.front();
            toVisit.pop();

            if (hasResource(resourceType, current, resourceEntity))
            {
                return resourceEntity;
            }

            if (distance >= maxRadius)
            {
                continue; // Stop expanding beyond max radius
            }

            for (const auto& dir : directions)
            {
                Vec2d neighbor{current.x + dir.x, current.y + dir.y};

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