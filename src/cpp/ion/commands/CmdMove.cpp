#include "CmdMove.h"

#include "Coordinates.h"
#include "EventPublisher.h"
#include "GameState.h"
#include "PathFinderBase.h"
#include "Player.h"
#include "Rect.h"
#include "ServiceRegistry.h"
#include "components/CompAction.h"
#include "components/CompAnimation.h"
#include "components/CompBuilding.h"
#include "components/CompDirty.h"
#include "components/CompGraphics.h"
#include "components/CompResource.h"
#include "components/CompTransform.h"
#include "components/CompUnit.h"
#include "debug.h"
#include "utils/Logger.h"
#include "utils/ObjectPool.h"

using namespace ion;

void CmdMove::onStart()
{
}

/**
 * @brief Handles the logic for queuing a move command for an entity.
 *
 * This function determines the target position for the entity to move towards (in case
 * the target was set to a particular entity build not direct position),
 * based on whether the target entity is a building or a resource. It calculates
 * the closest edge of the target entity's land area and finds a path to that position.
 * If a valid path is found, it sets the next intermediate goal for movement.
 * In debug builds, it overlays a visual marker at the target position for debugging purposes.
 *
 * Side Effects:
 * - Updates `targetPos`, `path`, and `nextIntermediateGoal` member variables.
 * - In debug mode, modifies debug overlays for the target tile entity.
 * - Retrieves and stores the coordinates service.
 */
void CmdMove::onQueue()
{
    if (targetEntity != entt::null && m_gameState->hasComponent<CompBuilding>(targetEntity))
    {
        auto [building, buildingTransform] =
            m_gameState->getComponents<CompBuilding, CompTransform>(targetEntity);
        auto rect = building.getLandInFeetRect(buildingTransform.position);
        auto& transform = m_gameState->getComponent<CompTransform>(m_entityID);

        targetPos = findClosestEdgeOfStaticEntity(targetEntity, transform.position, rect);
    }
    else if (targetEntity != entt::null && m_gameState->hasComponent<CompResource>(targetEntity))
    {
        auto [resource, resourceTransform] =
            m_gameState->getComponents<CompResource, CompTransform>(targetEntity);
        auto rect = resource.getLandInFeetRect(resourceTransform.position);
        auto& transform = m_gameState->getComponent<CompTransform>(m_entityID);

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
        auto tileEntity = m_gameState->gameMap.getEntity(MapLayerType::GROUND, targetPos.toTile());
        auto [graphics, dirty] = m_gameState->getComponents<CompGraphics, CompDirty>(tileEntity);

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

/**
 * Executes the move command for the entity associated with this command.
 *
 * This method checks if the target position is valid. If so, it animates the entity, and attempts
 * to move it. If the target position is invalid, returns true marking the sudden end of command.
 *
 * @param deltaTimeMs The elapsed time in milliseconds since the last update.
 * @param subCommands A list to which any sub-commands generated during execution can be added.
 * @return true if the movement executed successfully or if the target position is invalid;
 *         false if the move operation failed.
 */
bool CmdMove::onExecute(int deltaTimeMs, std::list<Command*>& subCommands)
{
    if (targetPos.isNull() == false) [[likely]]
    {
        auto [transform, action, animation, dirty] =
            m_gameState->getComponents<CompTransform, CompAction, CompAnimation, CompDirty>(
                m_entityID);

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

    auto ticksPerFrame = m_settings->getTicksPerSecond() / actionAnimation.value().speed;
    if (s_totalTicks % ticksPerFrame == 0)
    {
        dirty.markDirty(entityId);
        animation.frame++;
        animation.frame %= actionAnimation.value().frames;
    }
}

/**
 * @brief Moves the unit towards its target position, handling path following and collision
 * avoidance.
 *
 * This function updates the unit's position based on its current path, applying separation and
 * avoidance forces to resolve and avoid collisions. It checks if the target is reached, refines the
 * path as necessary, and updates the next intermediate goal. Debug overlays are updated in debug
 * builds to visualize movement and forces.
 *
 * @param transform Reference to the unit's transform component, containing position, speed, and
 * orientation.
 * @param deltaTimeMs Time elapsed since the last update, in milliseconds.
 * @return true if the move command is completed (target reached), false otherwise.
 */
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
            auto& debugOverlays = m_gameState->getComponent<CompGraphics>(m_entityID).debugOverlays;

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

/**
 * Calculates the new position of the unit based on its current transform and elapsed time.
 *
 * This function updates the position using the unit's speed, rotation (in degrees),
 * and the elapsed time in milliseconds. The rotation determines the direction of movement.
 *
 * @param transform Reference to the object's current transform, containing position, speed, and
 * rotation.
 * @param timeMs Elapsed time in milliseconds over which to apply the movement.
 * @return The new position as a Feet object after applying the movement.
 */
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

/**
 * @brief Updates the position of the unit and handles tile movement logic.
 *
 * This function sets the new position of the unit.
 * If the unit moves to a different tile, it updates the game map by removing the unit from
 * the old tile and adding it to the new tile. It also publishes a unit tile movement event and
 * refines the movement path. In debug builds, it updates the debug overlay arrow to visualize
 * the movement direction.
 *
 * @param transform Reference to the entity's transform component, which holds its position and
 * velocity.
 * @param newPosFeet The new position for the entity, specified in feet.
 */
void CmdMove::setPosition(CompTransform& transform, const Feet& newPosFeet)
{
    auto oldTile = transform.position.toTile();
    auto newTile = newPosFeet.toTile();

#ifndef NDEBUG
    m_gameState->getComponent<CompGraphics>(m_entityID).debugOverlays[0].arrowEnd =
        coordinates->feetToScreenUnits((newPosFeet + (transform.getVelocityVector() * 2)));
#endif

    if (oldTile != newTile)
    {
        m_gameState->gameMap.removeEntity(MapLayerType::UNITS, oldTile, m_entityID);
        m_gameState->gameMap.addEntity(MapLayerType::UNITS, newTile, m_entityID);

        publishEvent(Event::Type::UNIT_TILE_MOVEMENT,
                     UnitTileMovementData{m_entityID, newTile, transform.position});

        refinePath();
    }
    transform.position = newPosFeet;
}

/**
 * @brief Determines if there is a clear line of sight from the unit to the target position.
 *
 * This function checks whether the path from the unit's current position to the specified target
 * position is unobstructed by any static obstacles on the game map.
 *
 * @param target The target position to check line of sight to.
 * @return true if there are no static obstacles between the entity and the target; false otherwise.
 */
bool CmdMove::hasLineOfSight(const Feet& target)
{
    auto& transform = m_gameState->getComponent<CompTransform>(m_entityID);
    return m_gameState->gameMap.intersectsStaticObstacle(transform.position, target) == false;
}

/**
 * @brief Refines the current movement path by removing waypoints that are directly visible.
 *
 * This method iterates through the waypoints in the path and checks if each waypoint
 * is in line of sight using hasLineOfSight(). If a waypoint is visible, it is considered
 * for removal.
 * All waypoints up to (but not including) the last visible waypoint are removed
 * from the front of the path.
 *
 * If the path is empty or contains fewer than two waypoints, no refinement is performed.
 */
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

/**
 * @brief Finds a path from the uint's current position to the specified end position.
 *
 * This function uses the game's pathfinding system to compute a path for the given unit
 * from its current position to the target position specified by endPosInFeet. If a path is found,
 * it returns a list of positions (in Feet) representing the path, including the end position.
 * If no path is found, it logs an error and returns an empty list.
 *
 * @param endPosInFeet The target position to move to, in Feet.
 * @param entity The entity for which the path is being found.
 * @return std::list<Feet> The computed path as a list of Feet positions, or an empty list if no
 * path is found.
 */
std::list<Feet> CmdMove::findPath(const Feet& endPosInFeet, uint32_t entity)
{
    auto& transform = m_gameState->getComponent<CompTransform>(entity);
    Tile startPos = transform.position.toTile();
    auto endPos = endPosInFeet.toTile();

    TileMap map = m_gameState->gameMap;
    std::vector<Feet> newPath =
        m_gameState->getPathFinder()->findPath(map, transform.position, endPosInFeet);

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
        //             ServiceRegistry::getInstance().getService<GameState>()->getComponents<CompDirty,
        //             CompGraphics>(entity);
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

/**
 * @brief Resolves collisions for the unit by checking static and dynamic obstacles.
 *
 * This function first checks if the target tile is statically occupied and returns zero avoidance
 * if blocked (i.e. can't avoid). Then, it searches the surrounding tiles for dynamic entities
 * (units) that may cause a collision. For each detected collision, it calculates a repulsion
 * direction and accumulates avoidance vectors. The final avoidance vector is scaled and returned to
 * help the unit avoid overlapping with others.
 *
 * @param transform The transform component of the unit, containing position and collision radius.
 * @return Feet The average avoidance vector to resolve detected collisions.
 */
Feet CmdMove::resolveCollision(CompTransform& transform)
{
    // Static collision resolution
    auto newTilePos = transform.position.toTile();
    auto& gameMap = m_gameState->gameMap;
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

                auto& otherTransform = m_gameState->getComponent<CompTransform>(e);
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

/**
 * Calculates an avoidance force to prevent collision with nearby units.
 *
 * This function casts a ray in the direction of the unit's velocity to detect potential collisions
 * with other units within half the unit's line of sight. If a collision is detected, it computes
 * a force vector to steer the unit away from the other entity, based on their relative positions
 * and collision radii.
 *
 * @param transform The transform component of the current unit, containing position and velocity.
 * @return A Feet vector representing the avoidance force. Returns (0, 0) if no collision is
 * detected.
 */
Feet CmdMove::avoidCollision(CompTransform& transform)
{
    auto& unit = m_gameState->getComponent<CompUnit>(m_entityID);
    auto dir = transform.getVelocityVector().normalized();

    auto rayEnd = transform.position + (dir * (unit.lineOfSight / 2));
    auto unitToAvoid = intersectsUnits(m_entityID, transform, transform.position, rayEnd);

#ifndef NDEBUG
    m_gameState->getComponent<CompGraphics>(m_entityID).debugOverlays[3].arrowEnd =
        coordinates->feetToScreenUnits(rayEnd);
#endif

    if (unitToAvoid != entt::null)
    {
        spam("Posible collision with entity {}", unitToAvoid);
        auto& otherTransform = m_gameState->getComponent<CompTransform>(unitToAvoid);

        Feet dir = otherTransform.position - transform.position;
        auto forward = dir.normalized();
        Feet left(-forward.y, forward.x);

        Feet avoidanceForce = left * (otherTransform.collisionRadius * 10);
        return avoidanceForce;
    }
    return Feet(0, 0);
}

/**
 * Calculates the shortest distance from a point `q` to a line segment defined by points `p0` and
 * `p1`.
 *
 * If the segment is degenerate (i.e., `p0` and `p1` are the same), returns the distance from `q` to
 * `p0`. Otherwise, projects `q` onto the segment and returns the distance from `q` to the closest
 * point on the segment.
 *
 * @param p0 The starting point of the segment.
 * @param p1 The ending point of the segment.
 * @param q  The point from which the distance to the segment is measured.
 * @return   The shortest distance from `q` to the segment `[p0, p1]`.
 */
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

/**
 * Checks if the movement from `start` to `end` intersects with any units on the map,
 * excluding the unit identified by `self`.
 *
 * The function samples points along the movement path at intervals of ¼ tile and checks
 * for collisions with other units based on their collision radii.
 *
 * @param self The entity ID of the moving unit (to be excluded from collision checks).
 * @param transform The transform component of the moving unit, containing position and collision
 * radius.
 * @param start The starting position of the movement (in feet).
 * @param end The ending position of the movement (in feet).
 * @return The entity ID of the first unit intersected, or `entt::null` if no collision occurs.
 */
uint32_t CmdMove::intersectsUnits(uint32_t self,
                                  CompTransform& transform,
                                  const Feet& start,
                                  const Feet& end) const
{
    auto& gameMap = m_gameState->gameMap;

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
                    auto& otherTransform = m_gameState->getComponent<CompTransform>(otherUnit);
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

/**
 * @brief Checks if a line segment intersects with a circle.
 *
 * Given two endpoints of a line segment (p1 and p2), the center of a circle, and its radius,
 * this function determines whether the segment intersects the circle.
 *
 * @param p1 The starting point of the line segment.
 * @param p2 The ending point of the line segment.
 * @param center The center of the circle.
 * @param radius The radius of the circle.
 * @return true if the line segment intersects the circle, false otherwise.
 */
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

/**
 * @brief Finds the closest edge point of a static entity's bounding rectangle to a given position.
 *
 * This function computes the point on the edge of the specified static entity's rectangular area
 * (defined by `land`) that is closest to the provided position (`fromPos`). The result is clamped
 * within the bounds of the rectangle, effectively projecting the position onto the nearest edge.
 *
 * @param staticEntity The identifier of the static entity whose edge is being queried.
 * @param fromPos The position from which to find the closest edge point.
 * @param land The bounding rectangle of the static entity.
 * @return Feet The closest point on the edge of the rectangle to `fromPos`.
 */
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

/**
 * @brief Checks if a circular area around a unit overlaps with a rectangular building area.
 *
 * This function determines whether a circle, defined by the unit's position (`unitPos`)
 * and a squared radius (`radiusSq`), overlaps with a rectangle (`buildingRect`).
 * It calculates the closest point on the rectangle to the unit and checks if the distance
 * from the unit to this point is less than or equal to the circle's radius.
 *
 * @param unitPos The position of the unit as a Feet object.
 * @param radiusSq The squared radius of the unit's circular area.
 * @param buildingRect The rectangle representing the building area.
 * @return true if the circular area overlaps with the rectangle, false otherwise.
 */
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

/**
 * @brief Checks if the target is close enough to the unit.
 *
 * This function determines whether the unit has reached its target.
 * If the target is another entity, it checks if the entity's position and goal radius
 * overlap with the target entity's building land rectangle.
 * If the target is a position, it checks if the squared distance to the target position
 * is less than the entity's goal radius squared.
 *
 * @return true if the target is close enough; false otherwise.
 */
bool CmdMove::isTargetCloseEnough()
{
    auto& transformMy = m_gameState->getComponent<CompTransform>(m_entityID);

    if (targetEntity != entt::null)
    {
        if (m_gameState->hasComponent<CompBuilding>(targetEntity))
        {
            auto [transform, building] =
                m_gameState->getComponents<CompTransform, CompBuilding>(targetEntity);
            auto pos = transformMy.position;
            auto radiusSq = transformMy.goalRadiusSquared;
            auto rect = building.getLandInFeetRect(transform.position);

            return overlaps(pos, radiusSq, rect);
        }
        else if (m_gameState->hasComponent<CompResource>(targetEntity))
        {
            auto [transform, resource] =
                m_gameState->getComponents<CompTransform, CompResource>(targetEntity);
            auto pos = transformMy.position;
            auto radiusSq = transformMy.goalRadiusSquared;
            auto rect = resource.getLandInFeetRect(transform.position);

            return overlaps(pos, radiusSq, rect);
        }
        debug_assert(false, "Unknown entity type for target {}", targetEntity);
        return true;
    }
    else
    {
        return transformMy.position.distanceSquared(targetPos) < transformMy.goalRadiusSquared;
    }
}