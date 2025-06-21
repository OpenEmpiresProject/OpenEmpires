#include "CmdMove.h"

#include "EventPublisher.h"
#include "GameState.h"
#include "ServiceRegistry.h"
#include "components/CompGraphics.h"
#include "debug.h"
#include "utils/Logger.h"
#include "utils/ObjectPool.h"

using namespace ion;

void CmdMove::onStart()
{
}

void CmdMove::onQueue(uint32_t entityID)
{
    entity = entityID;
    path = findPath(goal, entityID);
    coordinates = ServiceRegistry::getInstance().getService<Coordinates>();
}

bool CmdMove::onExecute(uint32_t entityID, int deltaTimeMs)
{
    auto [transform, action, animation, dirty] =
        GameState::getInstance().getComponents<CompTransform, CompAction, CompAnimation, CompDirty>(
            entityID);

    animate(action, animation, dirty, deltaTimeMs, entityID);
    return move(transform, deltaTimeMs);
}

std::string CmdMove::toString() const
{
    return "move";
}

void CmdMove::destroy()
{
    ObjectPool<CmdMove>::release(this);
}

bool CmdMove::onCreateSubCommands(std::list<Command*>& subCommands)
{
    return false;
}

void CmdMove::animate(CompAction& action,
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

bool CmdMove::move(CompTransform& transform, int deltaTimeMs)
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
            spdlog::debug("Next hop {} reached", nextPos.toString());
            path.pop_front();
        }
        else
        {
            transform.face(nextPos);
            auto beforePos = transform.position;
            auto newPos = calculateNewPosition(transform, deltaTimeMs);
            if (resolveCollision(newPos))
            {
            }
            else
            {
                spdlog::debug("Couldn't resolve collision, skiping to next point");
                // Try to skip it
                path.pop_front();
            }
        }
    }
    return path.empty();
}

Vec2d CmdMove::calculateNewPosition(CompTransform& transform, int timeMs)
{
    debug_assert(transform.speed > 0, "Speed must be greater than zero");
    // Update the position based on speed and time
    auto rad = (std::numbers::pi * transform.rotation) / 180;
    auto timeS = (double) timeMs / 1000.0;

    auto newPos = transform.position;

    newPos.y -= transform.speed * std::cos(rad) * timeS;
    newPos.x += transform.speed * std::sin(rad) * timeS;

    return newPos;
}

void CmdMove::setPosition(CompTransform& transform, const Vec2d& newPosFeet)
{
    auto oldTile = coordinates->feetToTiles(transform.position);
    auto newTile = coordinates->feetToTiles(newPosFeet);

    if (oldTile != newTile)
    {
        GameState::getInstance().gameMap.removeEntity(MapLayerType::UNITS, oldTile, entity);
        GameState::getInstance().gameMap.addEntity(MapLayerType::UNITS, newTile, entity);

        publishEvent(Event::Type::UNIT_TILE_MOVEMENT,
                     UnitTileMovementData{entity, newTile, transform.position});
    }
    transform.position = newPosFeet;
}

std::list<Vec2d> CmdMove::findPath(const Vec2d& endPosInFeet, uint32_t entity)
{
    auto& transform = GameState::getInstance().getComponent<CompTransform>(entity);
    Vec2d startPos = transform.position;

    startPos = coordinates->feetToTiles(startPos);
    auto endPos = coordinates->feetToTiles(endPosInFeet);

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
            auto entity = map.getEntity(MapLayerType::GROUND, node);
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
            pathList.push_back(coordinates->getTileCenterInFeet(path[i]));
        }
        pathList.push_back(endPosInFeet);

        return pathList;
    }
    return std::list<Vec2d>();
}

constexpr int square(int n)
{
    return n * n;
}

bool CmdMove::resolveCollision(const Vec2d& newPosFeet)
{
    // Static collision resolution
    auto coordinateSystem = ServiceRegistry::getInstance().getService<Coordinates>();
    auto newTilePos = coordinateSystem->feetToTiles(newPosFeet);
    auto& state = GameState::getInstance();
    auto& transform = state.getComponent<CompTransform>(entity);
    auto& gameMap = state.gameMap;

    if (gameMap.isOccupied(MapLayerType::STATIC, newTilePos))
    {
        spdlog::debug("Tile {} is occupied", newTilePos.toString());
        // Can't resolve collision, target is blocked.
        return false;
    }

    if (gameMap.isOccupiedByAnother(MapLayerType::UNITS, newTilePos, entity))
    {
        // spdlog::debug("Tile {} is occupied by another entity", newTilePos.toString());
        return false;
    }

    // Dynamic collision resolution
    Vec2d searchStartTile = newTilePos - Vec2d(1, 1);
    Vec2d searchEndTile = newTilePos + Vec2d(1, 1);

    Vec2d totalAvoidance{0, 0};
    bool hasCollision = false;

    for (int x = searchStartTile.x; x <= searchEndTile.x; x++)
    {
        for (int y = searchStartTile.y; y <= searchEndTile.y; y++)
        {
            Vec2d gridPos{x, y};
            auto& entities = gameMap.getEntities(MapLayerType::UNITS, gridPos);

            for (auto e : entities)
            {
                if (e == entt::null || e == entity)
                    continue;

                // spdlog::debug("Checking collision with entity {}", e);

                auto& otherTransform = state.getComponent<CompTransform>(e);
                Vec2d toOther = otherTransform.position - newPosFeet;

                float distSq = toOther.lengthSquared();
                float minDistSq = square(transform.goalRadius + otherTransform.goalRadius);

                // spdlog::debug("distance: {}, min distance: {}", distSq, minDistSq);
                // spdlog::debug("oldPosFeet: {}", transform.position.toString());
                // spdlog::debug("newPosFeet: {}", newPosFeet.toString());
                // spdlog::debug("otherpos: {}", otherTransform.position.toString());
                // spdlog::debug("toOther: {}", toOther.toString());
                // spdlog::debug("toOther normalized10: {}", toOther.normalized10().toString());

                if (distSq < minDistSq && distSq > 0.0001f)
                {
                    hasCollision = true;

                    Vec2d repulsionDir = (-toOther).normalized10();
                    totalAvoidance += repulsionDir;

                    // spdlog::debug("totalAvoidance: {}", totalAvoidance.toString());
                }
            }
        }
    }

    if (hasCollision)
    {
        // Apply the integer push (scaled down if needed)
        Vec2d averagePush = totalAvoidance; // This keeps the actual push small

        // spdlog::debug("Collision detected! Avoidance vector {}", averagePush.toString());

        // You can apply as-is or convert to float if needed
        auto newPos = newPosFeet + averagePush;
        setPosition(transform, newPos);
    }
    else
    {
        setPosition(transform, newPosFeet);
    }

    return true;
}
