#ifndef CMDATTACK_H
#define CMDATTACK_H

#include "Coordinates.h"
#include "GameState.h"
#include "ServiceRegistry.h"
#include "Vec2d.h"
#include "commands/CmdMove.h"
#include "commands/Command.h"
#include "components/CompEntityInfo.h"
#include "components/CompResource.h"
#include "components/CompSelectible.h"
#include "components/CompTransform.h"
#include "utils/ObjectPool.h"

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
    }

    bool onExecute(uint32_t entityID, int deltaTimeMs) override
    {
        if (!isResourceAvailable())
        {
            if (!lookForAnotherTree())
            {
                // Could not find another tree, nothing to gather, done with the command
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
        int rounded = collectedWood;
        ;
        collectedWood -= rounded;

        if (rounded != 0)
        {
            cutTree(rounded);
        }
    }

    bool lookForAnotherTree()
    {
        spdlog::debug("Looking for another tree...");
        auto& transformMy = GameState::getInstance().getComponent<CompTransform>(entity);
        auto tilePos = coordinateSystem->feetToTiles(transformMy.position);

        auto otherResource = findClosestResource(tilePos, Constants::MAX_RESOURCE_LOOKUP_RADIUS);

        if (otherResource != entt::null)
        {
            spdlog::debug("Found resource {}", otherResource);
            target = otherResource;
            auto& transformTarget = GameState::getInstance().getComponent<CompTransform>(target);
            targetPosition = transformTarget.position;
        }
        return otherResource != entt::null;
    }

    bool hasResource(const Vec2d& posTile, uint32_t& resourceEntity)
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
                    resourceEntity = e;
                    return true;
                }
            }
        }
        return false;
    }

    uint32_t findClosestResource(const Vec2d& startTile, int maxRadius)
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

            if (hasResource(current, resourceEntity))
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

    bool cutTree(uint16_t delta)
    {
        auto tree = target;

        if (GameState::getInstance().hasComponent<CompResource>(tree))
        {
            auto [resource, dirty, info, select] =
                GameState::getInstance()
                    .getComponents<CompResource, CompDirty, CompEntityInfo, CompSelectible>(tree);
            resource.resource.amount =
                resource.resource.amount < delta ? 0 : resource.resource.amount - delta;
            if (resource.resource.amount == 0)
            {
                info.isDestroyed = true;
            }
            else
            {
                info.entitySubType = 1;
                info.variation = 0; //  regardless of the tree type, this is the chopped version
                // TODO: Might not be the most optimal way to bring down the bounding box a chopped
                // tree
                auto tw = Constants::TILE_PIXEL_WIDTH;
                auto th = Constants::TILE_PIXEL_HEIGHT;
                select.boundingBoxes[static_cast<int>(Direction::NONE)] =
                    Rect<int>(tw / 2, th / 2, tw, th);
            }
            spdlog::info("Tree has {} resources", resource.resource.amount);

            dirty.markDirty(tree);
            return info.isDestroyed;
        }
        else
        {
            spdlog::error("Target entity {} is not a resouce", target);
            return true;
        }
        return false;
    }
};

} // namespace ion

#endif