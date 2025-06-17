#ifndef CMDMOVE_H
#define CMDMOVE_H

#include "Coordinates.h"
#include "GameState.h"
#include "ServiceRegistry.h"
#include "Vec2d.h"
#include "commands/Command.h"
#include "components/CompAction.h"
#include "components/CompAnimation.h"
#include "components/CompDirty.h"
#include "components/CompTransform.h"
#include "utils/Logger.h"
#include "utils/ObjectPool.h"

#include <list>

namespace ion
{
class CmdMove : public Command
{
  public:
    Vec2d goal; // In Feet
    // TODO: this is temporary. need flow-field and goal position only.
    std::list<Vec2d> path;

  private:
    void onStart() override
    {
    }

    void onQueue(uint32_t entityID) override
    {
        path = findPath(goal, entityID);
    }

    bool onExecute(uint32_t entityID, int deltaTimeMs) override
    {
        auto [transform, action, animation, dirty] =
            GameState::getInstance()
                .getComponents<CompTransform, CompAction, CompAnimation, CompDirty>(entityID);

        animate(action, animation, dirty, deltaTimeMs, entityID);
        return move(transform, deltaTimeMs);
    }

    std::string toString() const override
    {
        return "move";
    }

    void destroy() override
    {
        ObjectPool<CmdMove>::release(this);
    }

    bool onCreateSubCommands(std::list<Command*>& subCommands) override
    {
        return false;
    }

    void animate(CompAction& action,
                 CompAnimation& animation,
                 CompDirty& dirty,
                 int deltaTimeMs,
                 uint32_t entityId)
    {
        action.action = Actions::MOVE;
        auto& actionAnimation = animation.animations[action.action];

        auto ticksPerFrame = m_settings->getTicksPerSecond() / actionAnimation.speed;
        if (s_totalTicks % ticksPerFrame == 0)
        {
            dirty.markDirty(entityId);
            animation.frame++;
            animation.frame %= actionAnimation.frames;
        }
    }

    bool move(CompTransform& transform, int deltaTimeMs)
    {
        if (!path.empty())
        {
            auto& nextPos = path.front();
            if (transform.position.distanceSquared(goal) < transform.goalRadiusSquared)
            {
                spdlog::debug("Reached goal before hopping all points");
                return true;
            }
            else if (transform.position.distanceSquared(nextPos) < transform.goalRadiusSquared)
            {
                path.pop_front();
            }
            else
            {
                transform.face(nextPos);

                auto beforePos = transform.position;
                auto coordinateSystem = ServiceRegistry::getInstance().getService<Coordinates>();

                // Perform 3 attempts to move towards the goal with decreasing delta
                // distances
                for (size_t i = 1; i < 4; i++)
                {
                    transform.move(deltaTimeMs / i);
                    auto newTilePos = coordinateSystem->feetToTiles(transform.position);

                    // TODO: Might need a better obstacle avoidance system
                    if (GameState::getInstance().gameMap.isOccupied(MapLayerType::STATIC, newTilePos) == false)
                    {
                        return path.empty();
                    }
                    transform.position = beforePos;
                }
                // If the unit couldn't move after 3 attempts, it is the end of movement. Can't move forward
                // as it is blocked.
                transform.position = beforePos;
                return true;
            }
        }
        return path.empty();
    }

    std::list<Vec2d> findPath(const Vec2d& endPosInFeet, uint32_t entity)
    {
        auto coordinateSystem = ServiceRegistry::getInstance().getService<Coordinates>();
        auto& transform = GameState::getInstance().getComponent<CompTransform>(entity);
        Vec2d startPos = transform.position;

        startPos = coordinateSystem->feetToTiles(startPos);
        auto endPos = coordinateSystem->feetToTiles(endPosInFeet);

        GridMap map = GameState::getInstance().gameMap;
        std::vector<Vec2d> path =
            GameState::getInstance().getPathFinder()->findPath(map, startPos, endPos);

        if (path.empty())
        {
            spdlog::error("No path found from {} to {}", startPos.toString(), endPos.toString());
        }
        else
        {
            for (Vec2d node : path)
            {
                // map.map[node.x][node.y] = 2;
                auto entity = map.layers[MapLayerType::GROUND].cells[node.x][node.y].getEntity();
                if (entity != entt::null)
                {
                    // TODO: Should avoid manipulating CompGraphics directly
                    auto [dirty, gc] =
                        GameState::getInstance().getComponents<CompDirty, CompGraphics>(entity);
                    gc.debugOverlays.push_back({DebugOverlay::Type::FILLED_CIRCLE, Color::BLUE,
                                                DebugOverlay::FixedPosition::CENTER});
                    dirty.markDirty(entity);
                }
            }

            std::list<Vec2d> pathList;
            for (size_t i = 0; i < path.size(); i++)
            {
                pathList.push_back(coordinateSystem->getTileCenterInFeet(path[i]));
            }
            pathList.push_back(endPosInFeet);

            return pathList;
        }
        return std::list<Vec2d>();
    }
};
} // namespace ion

#endif