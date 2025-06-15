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
#include "components/CompSelectible.h"
#include "components/CompTransform.h"
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
    uint32_t entity = entt::null;
    Vec2d targetPosition; // TODO: Use when target is absent by the time this command execute
    const int choppingSpeed = 10; // 10 wood per second
    float collectedWood = 0;
    std::shared_ptr<Coordinates> coordinateSystem;
    Ref<Player> player;

    void onStart() override
    {
        spdlog::debug("Start gathering resource...");
        coordinateSystem = ServiceRegistry::getInstance().getService<Coordinates>();
    }

    void onQueue(uint32_t entityID) override
    {
        // TODO: Reset frame to zero (since this is a new command)
        entity = entityID;
        auto& transformTarget = GameState::getInstance().getComponent<CompTransform>(target);
        targetPosition = transformTarget.position;

        auto& playerComp = GameState::getInstance().getComponent<CompPlayer>(entity);
        player = playerComp.player;
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
            auto [transform, action, animation, dirty] =
                GameState::getInstance()
                    .getComponents<CompTransform, CompAction, CompAnimation, CompDirty>(entityID);

            animate(action, animation, dirty, deltaTimeMs, entityID);
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
            spdlog::debug("Target is not close enough, moving...");
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
        auto& transformTarget = GameState::getInstance().getComponent<CompTransform>(target);
        auto& transformMy = GameState::getInstance().getComponent<CompTransform>(entity);
        return transformMy.position.distanceSquared(transformTarget.position) <
               transformTarget.goalRadiusSquared;
    }

    void animate(CompAction& action,
                 CompAnimation& animation,
                 CompDirty& dirty,
                 int deltaTimeMs,
                 uint32_t entityId)
    {
        action.action = 2; // TODO: Not good
        auto& actionAnimation = animation.animations[action.action];

        auto ticksPerFrame = m_settings->getTicksPerSecond() / actionAnimation.speed;
        if (s_totalTicks % ticksPerFrame == 0)
        {
            dirty.markDirty(entityId);
            animation.frame++;
            animation.frame %= actionAnimation.frames;
        }
    }

    void gather(CompTransform& transform, int deltaTimeMs)
    {
        collectedWood += float(choppingSpeed) * deltaTimeMs / 1000.0f;
        uint32_t rounded = collectedWood;
        collectedWood -= rounded;

        if (rounded != 0)
        {
            if (GameState::getInstance().hasComponent<CompResource>(target))
            {
                auto [resource, dirty] =
                    GameState::getInstance().getComponents<CompResource, CompDirty>(target);

                auto actualDelta = std::min(resource.resource.amount, rounded);
                resource.resource.amount -= actualDelta;
                player->grantResource(resource.resource.type, actualDelta);
                dirty.markDirty(target);
            }
            else
            {
                spdlog::error("Target entity {} is not a resouce", target);
            }
        }
    }

    bool lookForAnotherSimilarResource()
    {
        spdlog::debug("Looking for another resource...");
        auto& transformMy = GameState::getInstance().getComponent<CompTransform>(entity);
        auto tilePos = coordinateSystem->feetToTiles(transformMy.position);

        auto& resource = GameState::getInstance().getComponent<CompResource>(target);

        auto otherResource = findClosestResource(resource.resource.type, tilePos,
                                                 Constants::MAX_RESOURCE_LOOKUP_RADIUS);

        if (otherResource != entt::null)
        {
            spdlog::debug("Found resource {}", otherResource);
            target = otherResource;
            auto& transformTarget = GameState::getInstance().getComponent<CompTransform>(target);
            targetPosition = transformTarget.position;
        }
        return otherResource != entt::null;
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