#include "CmdMove.h"

#include "Coordinates.h"
#include "EventPublisher.h"
#include "PathFinderBase.h"
#include "Player.h"
#include "ProximityChecker.h"
#include "Rect.h"
#include "ServiceRegistry.h"
#include "Settings.h"
#include "StateManager.h"
#include "components/CompAction.h"
#include "components/CompAnimation.h"
#include "components/CompBuilding.h"
#include "components/CompGraphics.h"
#include "components/CompPlayer.h"
#include "components/CompResource.h"
#include "components/CompSelectible.h"
#include "components/CompTransform.h"
#include "components/CompUnit.h"
#include "debug.h"
#include "logging/Logger.h"
#include "utils/ObjectPool.h"

using namespace core;

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
    m_components->unit.formationSlot = FormationSlot();

    if (target.has_value())
    {
        // Enrich the target position
        if (target->entity.has_value())
        {
            auto targetEntity = target->entity.value();

            if (m_stateMan->hasComponent<CompUnit>(targetEntity))
            {
                auto& targetTransform = m_stateMan->getComponent<CompTransform>(targetEntity);
                auto targetPos = targetTransform.position;
                target.emplace(targetPos, Target::Type::UNIT, targetEntity);
            }
        }
    }
    else
    {
        spdlog::error("Target is not set for the unit {} to move", m_entityID);
        return;
    }

    if (not target->isValid())
    {
        spdlog::warn("Target is set but not valid for unit {} to move", m_entityID);
        return;
    }

    m_path = m_pathService->findPath(m_components->transform.position, target->pos,
                                     m_components->player.player);

    if (m_path.isEmpty())
    {
        spdlog::warn("Couldn't find path from {} to {} for unit",
                     m_components->transform.position.toString(), target->pos.toString(),
                     m_entityID);
    }
#ifndef NDEBUG
    // for (auto& pos : m_path.getWaypoints())
    //{
    //     auto tileEntity = m_stateMan->gameMap().getEntity(MapLayerType::GROUND,
    //     pos.toTile()); auto& graphics = m_stateMan->getComponent<CompGraphics>(tileEntity);

    //    if (not graphics.debugOverlays.empty())
    //    {
    //        //graphics.debugOverlays.clear();
    //        DebugOverlay filledCircle;
    //        filledCircle.type = DebugOverlay::Type::CIRCLE;
    //        filledCircle.color = Color::GREEN;
    //        filledCircle.absolutePosition = pos;
    //        graphics.debugOverlays.push_back(filledCircle);
    //        StateManager::markDirty(tileEntity);
    //    }
    //}

    // auto tileEntity = m_stateMan->gameMap().getEntity(MapLayerType::GROUND,
    // targetPos.toTile()); auto& graphics = m_stateMan->getComponent<CompGraphics>(tileEntity);

    // if (not graphics.debugOverlays.empty())
    //{
    //     //graphics.debugOverlays.clear();
    //     DebugOverlay filledCircle;
    //     filledCircle.type = DebugOverlay::Type::FILLED_CIRCLE;
    //     filledCircle.color = Color::RED;
    //     filledCircle.absolutePosition = targetPos;
    //     graphics.debugOverlays.push_back(filledCircle);
    //     StateManager::markDirty(tileEntity);
    // }
#endif
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
bool CmdMove::onExecute(int deltaTimeMs, int currentTick, std::list<Command*>& subCommands)
{
    if (not target.has_value() or not target->isValid()) [[unlikely]]
        return true;

    animate(deltaTimeMs, currentTick);
    return move(deltaTimeMs);
}

std::string CmdMove::toString() const
{
    return "move";
}

void CmdMove::destroy()
{
    ObjectPool<CmdMove>::release(this);
}

void CmdMove::animate(int deltaTimeMs, int currentTick)
{
    if (m_dontAnimate)
        return;

    m_components->action.action = actionOverride;
    const auto& actionAnimation = m_components->animation.animations[m_components->action.action];

    auto ticksPerFrame =
        int(m_settings->getTicksPerSecond() / (actionAnimation.speed * m_settings->getGameSpeed()));
    if (currentTick % ticksPerFrame == 0)
    {
        StateManager::markDirty(m_entityID);
        m_components->animation.frame++;
        m_components->animation.frame %= actionAnimation.frames;
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
bool CmdMove::move(int deltaTimeMs)
{
    /*
     *   Use the externally provided (by the parent command) arrival evaluator
     *   to see whether the unit is close enough according caller's requirements.
     */
    if (hasArrived())
    {
        return true;
    }

    if (!m_path.isEmpty()) [[likely]]
    {
        const auto& nextWaypoint = m_path.nextWaypoint();

        // If this is the last point of the path, unit is arriving at the target
        auto arriving = m_path.getWaypoints().size() == 1;

        if (isPositionCloseEnough(nextWaypoint, arriving))
        {
            auto waypointCopy = nextWaypoint;
            m_path.removeNextWaypoint();

            spam("Unit {} reached next hop {}. Remaining hops count {}", m_entityID,
                 waypointCopy.toString(), m_path.getWaypoints().size());
        }
        else
        {
            Feet finalDir = avoidCollision(deltaTimeMs, nextWaypoint);
            auto isIdle = stayIdleIfSeemsStuck(deltaTimeMs, finalDir);
            if (not isIdle)
            {
                updateDebugOverlays(finalDir);
                updateUnitPosition(deltaTimeMs, finalDir);
            }
        }
    }
    return m_path.isEmpty();
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
void CmdMove::updateUnitPosition(int deltaTimeMs, const Feet& forwardDir)
{
    auto timeS = (double) deltaTimeMs / 1000.0;

    const Feet newPos =
        m_components->transform.position +
        (forwardDir * (m_components->transform.speed * timeS * m_settings->getGameSpeed()));
    m_components->transform.face(newPos);

    const auto oldTile = m_components->transform.position.toTile();
    const auto newTile = newPos.toTile();

    if (oldTile != newTile)
    {
        m_stateMan->gameMap().removeEntity(MapLayerType::UNITS, oldTile, m_entityID);
        m_stateMan->gameMap().addEntity(MapLayerType::UNITS, newTile, m_entityID);

        publishEvent(Event::Type::UNIT_TILE_MOVEMENT,
                     UnitTileMovementData{m_entityID, newTile, m_components->transform.position});
    }
    m_components->transform.position = newPos;
}

Feet CmdMove::avoidCollision(int deltaTimeMs, const Feet& goalPos)
{
    auto preferredDir = goalPos - m_components->transform.position;
    auto deltaTimeS = (double) deltaTimeMs / 1000.0;
    const auto& currPos = m_components->transform.position;
    const auto& collisionRadius = m_components->transform.collisionRadius;
    auto speed = m_components->transform.speed;
    float lookaheadDuration = 1.0f;

    spam("{} Preferred moving direction {} from {} to goal {}", m_entityID,
         preferredDir.normalized().toString(), m_components->transform.position.toString(),
         goalPos.toString());

    auto quality = AvoidnaceQuality::MEDIUM;

    if (target->type == Target::Type::UNIT)
    {
        auto possibleDistanceCanTravel = speed * lookaheadDuration * m_settings->getGameSpeed();
        auto distance = m_components->transform.position.distance(target->pos);
        if ((distance - possibleDistanceCanTravel) < (collisionRadius * 2))
        {
            quality = AvoidnaceQuality::HIGH;
            lookaheadDuration = 0.5f;
        }
    }

    return m_pathService->getBestAvoidanceDirectionVector(currPos, preferredDir, collisionRadius,
                                                          speed, lookaheadDuration, m_entityID,
                                                          quality, target.value());
}

core::Command* CmdMove::clone()
{
    return ObjectPool<CmdMove>::acquire(*this);
}

bool CmdMove::isPositionCloseEnough(const Feet& pos, bool arriving) const
{
    auto distanceSq = m_components->transform.position.distanceSquared(pos);

    // When arriving at the target, reduce the collision radius to reach the
    // target more precisely.
    auto arrivingFactor = arriving ? 0.5f : 1.0f;

    return distanceSq < (collisionRadius * collisionRadius * arrivingFactor);
}

bool CmdMove::hasArrived()
{
    if (target->arrivalEvaluator.has_value())
    {
        return target->arrivalEvaluator.value()();
    }
    // For movement to be precise, we use reduced collision radius

    auto distanceSq = m_components->transform.position.distanceSquared(target->pos);

    return distanceSq < (collisionRadius * collisionRadius);
}

bool CmdMove::stayIdleIfSeemsStuck(int deltaTimeMs, const Feet& forwardDir)
{
    m_dontAnimate = false;

    if (forwardDir.dot(m_previousBestDirection) < 0.0f) // Direction flipped
    {
        m_directionFlipDurationMs += deltaTimeMs;
        m_numberOfDirectionFlips++;

        if (m_numberOfDirectionFlips >= DIRECTION_FLIP_THRESHOLD)
        {
            if (m_numberOfDirectionFlips == DIRECTION_FLIP_THRESHOLD)
            {
                spdlog::debug("Too many direction flips; {} waiting to settle down", m_entityID);
            }

            if (m_directionFlipDurationMs < DIRECTION_FLIP_WAIT_TIME_MS)
            {
                m_dontAnimate = true;
                return true;
            }
            else
            {
                spdlog::debug("{} resuming the movement after being stationary", m_entityID);
                m_numberOfDirectionFlips = 0;
                m_directionFlipDurationMs = 0;
            }
        }
    }
    else // Suggestion is either perpendicular or same direction
    {
        m_numberOfDirectionFlips = 0;
        m_directionFlipDurationMs = 0;
    }
    m_previousBestDirection = forwardDir;
    return false;
}

void CmdMove::updateDebugOverlays(const Feet& forwardDirection)
{

#ifndef NDEBUG
    auto& debugOverlays = m_stateMan->getComponent<CompGraphics>(m_entityID).debugOverlays;

    if (not debugOverlays.empty())
    {
        debugOverlays[0].arrowEnd = m_coordinates->feetToScreenUnits(
            (m_components->transform.position + (forwardDirection * 200)));

        auto tileFeetDiagonal = std::sqrt(2 * Constants::FEET_PER_TILE * Constants::FEET_PER_TILE);
        auto circleFeetRadius = m_components->transform.collisionRadius;
        debugOverlays[1].circlePixelRadius =
            circleFeetRadius * Constants::TILE_PIXEL_WIDTH / tileFeetDiagonal;
    }

#endif
}
