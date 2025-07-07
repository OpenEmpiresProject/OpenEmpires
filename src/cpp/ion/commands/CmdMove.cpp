#include "CmdMove.h"

#include "EventPublisher.h"
#include "GameState.h"
#include "ServiceRegistry.h"
#include "components/CompBuilding.h"
#include "components/CompGraphics.h"
#include "components/CompResource.h"
#include "components/CompUnit.h"
#include "debug.h"
#include "utils/Logger.h"
#include "utils/ObjectPool.h"

using namespace ion;

void CmdMove::onStart()
{
}

void CmdMove::onQueue()
{
    auto& gameState = GameState::getInstance();

    if (targetEntity != entt::null && gameState.hasComponent<CompBuilding>(targetEntity))
    {
        auto [building, buildingTransform] =
            Entity::getComponents<CompBuilding, CompTransform>(targetEntity);
        auto rect = building.getLandInFeetRect(buildingTransform.position);
        auto& transform = gameState.getComponent<CompTransform>(m_entityID);

        targetPos = findClosestEdgeOfStaticEntity(targetEntity, transform.position, rect);
    }
    else if (targetEntity != entt::null && gameState.hasComponent<CompResource>(targetEntity))
    {
        auto [resource, resourceTransform] =
            Entity::getComponents<CompResource, CompTransform>(targetEntity);
        auto rect = resource.getLandInFeetRect(resourceTransform.position);
        auto& transform = gameState.getComponent<CompTransform>(m_entityID);

        targetPos = findClosestEdgeOfStaticEntity(targetEntity, transform.position, rect);
    }

    if (targetPos.isNull() == false)
    {
        path = findPath(targetPos, m_entityID);
        refinePath();
        if (path.empty() == false)
            nextIntermediateGoal = path.front();
        else
            nextIntermediateGoal = Feet::null;

#ifndef NDEBUG
        auto tileEntity = gameState.gameMap.getEntity(MapLayerType::GROUND, targetPos.toTile());
        auto [graphics, dirty] = Entity::getComponents<CompGraphics, CompDirty>(tileEntity);

        graphics.debugOverlays.clear();
        DebugOverlay filledCircle;
        filledCircle.type = DebugOverlay::Type::FILLED_CIRCLE;
        filledCircle.color = Color::RED;
        filledCircle.absolutePosition = targetPos;
        graphics.debugOverlays.push_back(filledCircle);
        dirty.markDirty(tileEntity);
#endif
    }
    coordinates = ServiceRegistry::getInstance().getService<Coordinates>();
}

bool CmdMove::onExecute(int deltaTimeMs, std::list<Command*>& subCommands)
{
    if (targetPos.isNull() == false) [[likely]]
    {
        auto [transform, action, animation, dirty] =
            Entity::getComponents<CompTransform, CompAction, CompAnimation, CompDirty>(m_entityID);

        animate(action, animation, dirty, deltaTimeMs, m_entityID);
        return move(transform, deltaTimeMs);
    }
    else [[unlikely]]
    {
        spdlog::error("Target pos {} is not properly set to move.", targetPos.toString());
        return true;
    }
}

std::string CmdMove::toString() const
{
    return "move";
}

void CmdMove::destroy()
{
    ObjectPool<CmdMove>::release(this);
}

void CmdMove::animate(CompAction& action,
                      CompAnimation& animation,
                      CompDirty& dirty,
                      int deltaTimeMs,
                      uint32_t entityId)
{
    action.action = actionOverride;
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
    if (isTargetCloseEnough())
    {
        spdlog::debug("Target is reached, move command is completed");
        return true;
    }

    if (!path.empty())
    {
        debug_assert(nextIntermediateGoal != Feet::null, "Next intermediate target must be set");

        if (transform.position.distanceSquared(nextIntermediateGoal) < transform.goalRadiusSquared)
        {
            spdlog::debug("Next hop {} reached", nextIntermediateGoal.toString());

            if (path.empty() == false && path.front() == nextIntermediateGoal)
            {
                path.pop_front();
            }

            refinePath();
            if (path.empty() == false)
                nextIntermediateGoal = path.front();
            else
                nextIntermediateGoal = Feet::null;
        }
        else
        {

            auto desiredDirection = nextIntermediateGoal - transform.position;
            auto separationForce = resolveCollision(transform);
            auto avoidanceForce = avoidCollision(transform);

            Feet finalDir = desiredDirection + separationForce + avoidanceForce;

#ifndef NDEBUG
            auto& state = GameState::getInstance();
            auto& debugOverlays = state.getComponent<CompGraphics>(m_entityID).debugOverlays;

            if (separationForce != Feet(0, 0))
                debugOverlays[1].arrowEnd =
                    coordinates->feetToScreenUnits((transform.position + (separationForce)));
            else
                debugOverlays[1].arrowEnd = Vec2::zero;

            if (avoidanceForce != Feet(0, 0))
                debugOverlays[2].arrowEnd =
                    coordinates->feetToScreenUnits((transform.position + (avoidanceForce)));
            else
                debugOverlays[2].arrowEnd = Vec2::zero;

            debugOverlays[4].arrowEnd =
                coordinates->feetToScreenUnits((transform.position + (finalDir)));
#endif

            finalDir = finalDir.normalized();

            auto timeS = (double) deltaTimeMs / 1000.0;

            auto newPos = transform.position + (finalDir * (transform.speed * timeS));
            transform.face(newPos);

            setPosition(transform, newPos);
        }
    }
    return path.empty();
}

Feet CmdMove::calculateNewPosition(CompTransform& transform, int timeMs)
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

void CmdMove::setPosition(CompTransform& transform, const Feet& newPosFeet)
{
    auto oldTile = transform.position.toTile();
    auto newTile = newPosFeet.toTile();
    auto& state = GameState::getInstance();

#ifndef NDEBUG
    state.getComponent<CompGraphics>(m_entityID).debugOverlays[0].arrowEnd =
        coordinates->feetToScreenUnits((newPosFeet + (transform.getVelocityVector() * 2)));
#endif

    if (oldTile != newTile)
    {
        GameState::getInstance().gameMap.removeEntity(MapLayerType::UNITS, oldTile, m_entityID);
        GameState::getInstance().gameMap.addEntity(MapLayerType::UNITS, newTile, m_entityID);

        publishEvent(Event::Type::UNIT_TILE_MOVEMENT,
                     UnitTileMovementData{m_entityID, newTile, transform.position});

        refinePath();
    }
    transform.position = newPosFeet;
}

bool CmdMove::hasLineOfSight(const Feet& target)
{
    auto& state = GameState::getInstance();
    auto& transform = state.getComponent<CompTransform>(m_entityID);
    return state.gameMap.intersectsStaticObstacle(transform.position, target) == false;
}

void CmdMove::refinePath()
{
    if (path.empty() || path.size() < 2)
    {
        spam("Either path is empty or has less than 2 points. Nothing to refine");
        return;
    }
    auto length = path.size();

    int pointsToRemove = -1;

    for (auto waypoint : path)
    {
        if (hasLineOfSight(waypoint))
        {
            spam("Waypoint {} is in line of sight, consdered to remove", waypoint.toString());
            pointsToRemove++;
        }
        else
        {
            spam("Waypoint {} is NOT in line of sight", waypoint.toString());
            break;
        }
    }

    spam("Removing {} waypoints", pointsToRemove);

    for (int i = 0; i < pointsToRemove && !path.empty(); i++)
    {
        spam("Removing waypoint {}", path.front().toString());
        path.pop_front();
    }
}

std::list<Feet> CmdMove::findPath(const Feet& endPosInFeet, uint32_t entity)
{
    auto& transform = GameState::getInstance().getComponent<CompTransform>(entity);
    Tile startPos = transform.position.toTile();
    auto endPos = endPosInFeet.toTile();

    TileMap map = GameState::getInstance().gameMap;
    std::vector<Feet> newPath =
        GameState::getInstance().getPathFinder()->findPath(map, transform.position, endPosInFeet);

    if (newPath.empty())
    {
        spdlog::error("No path found from {} to {}", startPos.toString(), endPos.toString());
    }
    else
    {
        // for (Feet node : newPath)
        // {
        //     // map.map[node.x][node.y] = 2;
        //     auto entity = map.getEntity(MapLayerType::GROUND, node.toTile());
        //     if (entity != entt::null)
        //     {
        //         // TODO: Should avoid manipulating CompGraphics directly
        //         auto [dirty, gc] =
        //             GameState::getInstance().getComponents<CompDirty, CompGraphics>(entity);
        //         gc.debugOverlays.push_back({DebugOverlay::Type::FILLED_CIRCLE, Color::BLUE,
        //                                     DebugOverlay::FixedPosition::CENTER});
        //         dirty.markDirty(entity);
        //     }
        // }

        std::list<Feet> pathList;
        for (size_t i = 0; i < newPath.size(); i++)
        {
            pathList.push_back(newPath[i]);
        }
        pathList.push_back(endPosInFeet);

        return pathList;
    }
    return std::list<Feet>();
}

constexpr int square(int n)
{
    return n * n;
}

Feet CmdMove::resolveCollision(CompTransform& transform)
{
    // Static collision resolution
    auto newTilePos = transform.position.toTile();
    auto& state = GameState::getInstance();
    auto& gameMap = state.gameMap;
    Feet totalAvoidance{0, 0};

    if (gameMap.isOccupied(MapLayerType::STATIC, newTilePos))
    {
        spdlog::debug("Collision resolution. Tile {} is occupied", newTilePos.toString());
        // Can't resolve collision, target is blocked.
        return totalAvoidance;
    }

    // Dynamic collision resolution
    Tile searchStartTile = newTilePos - Tile(1, 1);
    Tile searchEndTile = newTilePos + Tile(1, 1);

    bool hasCollision = false;

    for (int x = searchStartTile.x; x <= searchEndTile.x; x++)
    {
        for (int y = searchStartTile.y; y <= searchEndTile.y; y++)
        {
            Tile gridPos{x, y};
            auto& entities = gameMap.getEntities(MapLayerType::UNITS, gridPos);

            for (auto e : entities)
            {
                if (e == entt::null || e == m_entityID)
                    continue;

                auto& otherTransform = state.getComponent<CompTransform>(e);
                Feet toOther = otherTransform.position - transform.position;

                float distSq = toOther.lengthSquared();
                float minDistSq =
                    square(transform.collisionRadius + otherTransform.collisionRadius);

                if (distSq < minDistSq && distSq > 0.0001f)
                {
                    hasCollision = true;

                    Feet repulsionDir = -toOther;
                    repulsionDir = repulsionDir.normalized();
                    totalAvoidance += repulsionDir;
                }
            }
        }
    }

    Feet averagePush = totalAvoidance * 2;

    return averagePush;
}

Feet CmdMove::avoidCollision(CompTransform& transform)
{
    auto& unit = GameState::getInstance().getComponent<CompUnit>(m_entityID);
    auto dir = transform.getVelocityVector().normalized();

    auto rayEnd = transform.position + (dir * (unit.lineOfSight / 2));
    auto unitToAvoid = intersectsUnits(m_entityID, transform, transform.position, rayEnd);

#ifndef NDEBUG
    auto& state = GameState::getInstance();
    state.getComponent<CompGraphics>(m_entityID).debugOverlays[3].arrowEnd =
        coordinates->feetToScreenUnits(rayEnd);
#endif

    if (unitToAvoid != entt::null)
    {
        spam("Posible collision with entity {}", unitToAvoid);
        auto& otherTransform = GameState::getInstance().getComponent<CompTransform>(unitToAvoid);

        Feet dir = otherTransform.position - transform.position;
        auto forward = dir.normalized();
        Feet left(-forward.y, forward.x);

        Feet avoidanceForce = left * (otherTransform.collisionRadius * 10);
        return avoidanceForce;
    }
    return Feet(0, 0);
}

double CmdMove::distancePointToSegment(const Feet& p0, const Feet& p1, const Feet& q) const
{
    const Feet v = p1 - p0;
    const Feet w = q - p0;

    const double lenSq = v.lengthSquared();
    if (lenSq == 0.0)
    {
        // Degenerate segment (start == end)
        return (q - p0).length();
    }

    double t = std::clamp(w.dot(v) / lenSq, 0.0, 1.0);
    auto vx = (double) (v.x) * t;
    auto vy = (double) (v.y) * t;

    Feet projection = p0 + Feet(vx, vy);

    spam("Start {}, end {}, q {}, projection {}", p0.toString(), p1.toString(), q.toString(),
         projection.toString());

    return (q - projection).length();
}

uint32_t CmdMove::intersectsUnits(uint32_t self,
                                  CompTransform& transform,
                                  const Feet& start,
                                  const Feet& end) const
{
    auto& state = GameState::getInstance();
    auto& gameMap = state.gameMap;

    float distance = start.distance(end);
    int numSteps =
        static_cast<int>(distance / (Constants::FEET_PER_TILE * 0.25f)); // Sample every ¼ tile

    if (numSteps <= 0)
        return false;

    Feet step = (end - start) / static_cast<float>(numSteps);

    for (int i = 0; i <= numSteps; ++i)
    {
        // TODO: Optimize this to avoid duplicated tile check
        Feet point = start + step * static_cast<float>(i);
        Tile tile = point.toTile();

        if (gameMap.isOccupied(MapLayerType::UNITS, tile))
        {
            auto& otherUnits = gameMap.getEntities(MapLayerType::UNITS, tile);
            for (auto otherUnit : otherUnits)
            {
                if (otherUnit != entt::null && otherUnit != self)
                {
                    auto& otherTransform =
                        GameState::getInstance().getComponent<CompTransform>(otherUnit);
                    double d = distancePointToSegment(start, end, otherTransform.position);
                    auto totalRadius = transform.collisionRadius + otherTransform.collisionRadius;

                    spam("Checking collision with entity {} at distance {}, LOS {}, projected "
                         "distance {}",
                         otherUnit, (otherTransform.position - start).length(),
                         (start - end).length(), d);

                    if (d <= totalRadius)
                    {
                        return otherUnit;
                    }
                }
            }
        }
    }
    return entt::null; // Clear line
}

bool CmdMove::lineIntersectsCircle(const Vec2& p1,
                                   const Vec2& p2,
                                   const Vec2& center,
                                   float radius) const
{
    // Vector from p1 to p2
    Vec2 d = p2 - p1;
    // Vector from p1 to circle center
    Vec2 f = p1 - center;

    float a = d.dot(d);
    float b = 2 * f.dot(d);
    float c = f.dot(f) - radius * radius;

    float discriminant = b * b - 4 * a * c;
    if (discriminant < 0)
    {
        // No intersection
        return false;
    }

    discriminant = std::sqrt(discriminant);
    float t1 = (-b - discriminant) / (2 * a);
    float t2 = (-b + discriminant) / (2 * a);

    // Check if either t is in [0, 1] → segment intersects circle
    return (t1 >= 0 && t1 <= 1) || (t2 >= 0 && t2 <= 1);
}

Feet CmdMove::findClosestEdgeOfStaticEntity(uint32_t staticEntity,
                                            const Feet& fromPos,
                                            const Rect<float>& land)
{
    float x_min = land.x;
    float x_max = land.x + land.w;
    float y_min = land.y;
    float y_max = land.y + land.h;

    float closestX = std::clamp(fromPos.x, x_min, x_max);
    float closestY = std::clamp(fromPos.y, y_min, y_max);

    return Feet(closestX, closestY);
}

bool CmdMove::overlaps(const Feet& unitPos, float radiusSq, const Rect<float>& buildingRect)
{
    float x_min = buildingRect.x;
    float x_max = buildingRect.x + buildingRect.w;
    float y_min = buildingRect.y;
    float y_max = buildingRect.y + buildingRect.h;

    float closestX = std::clamp(unitPos.x, x_min, x_max);
    float closestY = std::clamp(unitPos.y, y_min, y_max);

    float dx = unitPos.x - closestX;
    float dy = unitPos.y - closestY;

    return (dx * dx + dy * dy) <= radiusSq;
}

bool CmdMove::isTargetCloseEnough()
{
    auto& transformMy = Entity::getComponent<CompTransform>(m_entityID);

    if (targetEntity != entt::null)
    {
        auto [transform, building] =
            Entity::getComponents<CompTransform, CompBuilding>(targetEntity);
        auto pos = transformMy.position;
        auto radiusSq = transformMy.goalRadiusSquared;
        auto rect = building.getLandInFeetRect(transform.position);

        return overlaps(pos, radiusSq, rect);
    }
    else
    {
        return transformMy.position.distanceSquared(targetPos) < transformMy.goalRadiusSquared;
    }
}