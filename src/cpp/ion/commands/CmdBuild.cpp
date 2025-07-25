#include "CmdBuild.h"

#include "Feet.h"
#include "GameState.h"
#include "Rect.h"
#include "commands/CmdMove.h"
#include "components/CompAction.h"
#include "components/CompAnimation.h"
#include "components/CompBuilder.h"
#include "components/CompBuilding.h"
#include "components/CompDirty.h"
#include "components/CompTransform.h"
#include "debug.h"
#include "utils/Logger.h"
#include "utils/ObjectPool.h"

using namespace ion;

void CmdBuild::onStart()
{
}

void CmdBuild::onQueue()
{
    personalBuildingProgress = 0;
}

/**
 * @brief Executes the build command logic for the current frame.
 *
 * This method checks if the build target is close enough. If so, it performs
 * animation and building actions for the given time delta. Otherwise, it issues
 * movement commands to get closer to the target. Sub-commands may be added to
 * the provided list as needed.
 *
 * @param deltaTimeMs Time elapsed since the last execution, in milliseconds.
 * @param subCommands List to which any required sub-commands may be appended.
 * @return true if the build command is complete; false otherwise.
 */
bool CmdBuild::onExecute(int deltaTimeMs, std::list<Command*>& subCommands)
{
    if (isCloseEnough())
    {
        animate(deltaTimeMs);
        build(deltaTimeMs);
    }
    else
    {
        moveCloser(subCommands);
    }
    return isComplete();
}

std::string CmdBuild::toString() const
{
    return "build";
}

void CmdBuild::destroy()
{
    ObjectPool<CmdBuild>::release(this);
}

/**
 * @brief Animates the building action for the unit.
 *
 * Overrides the unit's action to BUILDING and increments the animation frame
 * based on the animation speed and elapsed ticks. Marks the entity as dirty when
 * the frame is updated to ensure changes are processed. The animation frames loop
 * continuously, making the building action repeatable.
 *
 * @param deltaTimeMs The elapsed time in milliseconds since the last tick.
 */
void CmdBuild::animate(int deltaTimeMs)
{
    m_components->action.action = UnitAction::BUILDING;
    auto& actionAnimation = m_components->animation.animations[m_components->action.action];

    auto ticksPerFrame = m_settings->getTicksPerSecond() / actionAnimation.value().speed;
    if (s_totalTicks % ticksPerFrame == 0)
    {
        m_components->dirty.markDirty(m_entityID);
        m_components->animation.frame++;
        m_components->animation.frame %= actionAnimation.value().frames; // Building is repeatable
    }
}

/**
 * @brief Determines if the builder is close enough to the target building location.
 *
 * This function checks whether the builder's goal radius overlap with the
 * rectangular area designated for the building.
 *
 * @return true if the builder is close enough to the target building location; false otherwise.
 */
bool CmdBuild::isCloseEnough()
{
    debug_assert(target != entt::null, "Proposed entity to build is null");

    auto [transform, building] = m_gameState->getComponents<CompTransform, CompBuilding>(target);
    auto pos = m_components->transform.position;
    auto radiusSq = m_components->transform.goalRadiusSquared;
    auto rect = building.getLandInFeetRect(transform.position);

    return overlaps(pos, radiusSq, rect);
}

/**
 * @brief Checks if the building construction is complete.
 *
 * This function returns true if the construction progress of the target building
 * has reached 100%, indicating that the building is fully constructed.
 *
 * @return true if construction is complete, false otherwise.
 */
bool CmdBuild::isComplete()
{
    return m_gameState->getComponent<CompBuilding>(target).constructionProgress == 100;
}

/**
 * @brief Advances the construction progress of a building by the builder's build speed.
 *
 * This function calculates the amount of construction progress to add based on the builder's
 * build speed and the elapsed time in milliseconds and updates the building's construction
 * progress. Also marks the building dirty with a lesser frequency to let the BuildingManager
 * picks up and updates the in-progress building graphic.
 *
 * @param deltaTimeMs The elapsed time in milliseconds since the last update.
 */
void CmdBuild::build(int deltaTimeMs)
{
    auto [building, dirty] = m_gameState->getComponents<CompBuilding, CompDirty>(target);
    auto& builder = m_gameState->getComponent<CompBuilder>(m_entityID);

    personalBuildingProgress += float(builder.buildSpeed) * deltaTimeMs / 1000.0f;
    uint32_t rounded = personalBuildingProgress;
    personalBuildingProgress -= rounded;

    if (rounded != 0)
    {
        building.constructionProgress += rounded;
        if (building.constructionProgress >= 100)
        {
            building.constructionProgress = 100;
        }

        if (building.constructionProgress % 10 == 0)
            dirty.markDirty(target);
    }
}

/**
 * @brief Moves the builder closer to the target if it is not close enough to build.
 *
 * This function creates a move command to approach the target. The move command is created with
 * a lightly higher priority and added to the list of sub-commands for execution.
 *
 * @param subCommands A list of sub-commands to which the move command will be appended.
 */
void CmdBuild::moveCloser(std::list<Command*>& subCommands)
{
    debug_assert(target != entt::null, "Proposed entity to build is null");

    auto targetPosition = m_gameState->getComponent<CompTransform>(target).position;

    spdlog::debug("Target {} at {} is not close enough to build, moving...", target,
                  targetPosition.toString());
    auto move = ObjectPool<CmdMove>::acquire();
    move->targetEntity = target;
    move->setPriority(getPriority() + CHILD_PRIORITY_OFFSET);
    subCommands.push_back(move);
}

/**
 * @brief Checks if a circular area around a unit overlaps with a rectangular building area.
 *
 * This function determines whether a circle, defined by the unit's position (`unitPos`)
 * and a squared radius (`radiusSq`), intersects or touches the rectangle specified by
 * `buildingRect`. It calculates the closest point on the rectangle to the unit's position,
 * then checks if the distance between this point and the unit's position is less than or
 * equal to the circle's radius.
 *
 * @param unitPos The position of the unit (center of the circle).
 * @param radiusSq The squared radius of the unit's area.
 * @param buildingRect The rectangle representing the building's area.
 * @return true if the circle overlaps or touches the rectangle, false otherwise.
 */
bool CmdBuild::overlaps(const Feet& unitPos, float radiusSq, const Rect<float>& buildingRect)
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
