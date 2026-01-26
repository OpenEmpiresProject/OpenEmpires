#ifndef CMDDROPRESOURCE_H
#define CMDDROPRESOURCE_H

#include "Coordinates.h"
#include "Feet.h"
#include "Player.h"
#include "Rect.h"
#include "ServiceRegistry.h"
#include "StateManager.h"
#include "commands/CmdMove.h"
#include "commands/Command.h"
#include "components/CompAction.h"
#include "components/CompAnimation.h"
#include "components/CompBuilding.h"
#include "components/CompEntityInfo.h"
#include "components/CompPlayer.h"
#include "components/CompResourceGatherer.h"
#include "components/CompTransform.h"
#include "debug.h"
#include "utils/ObjectPool.h"

#include <algorithm>

namespace core
{
class CmdDropResource : public Command
{
  public:
    uint8_t resourceType = Constants::RESOURCE_TYPE_NONE;

  private:
    uint32_t m_dropOffEntity = entt::null;
    CompResourceGatherer* m_gatherer = nullptr;

  private:
    std::string toString() const override
    {
        return "drop-resource";
    }

    Command* clone() override
    {
        return ObjectPool<CmdDropResource>::acquire(*this);
    }

    void destroy() override
    {
        ObjectPool<CmdDropResource>::release(this);
    }

    void onStart() override
    {
        spdlog::debug("Start dropping resource...");
        animate();
    }

    void onQueue() override
    {
        m_gatherer = &(m_stateMan->getComponent<CompResourceGatherer>(m_entityID));
    }

    /**
     * @brief Executes the drop resource command logic.
     *
     * This method attempts to find the closest drop-off building for the resource.
     * If the drop-off building is close enough, it drops the resource and returns true
     * marking the end of the command execution.
     * Otherwise, it issues a command to move towards the drop-off building and returns false.
     *
     * @param deltaTimeMs The elapsed time in milliseconds since the last execution.
     * @param subCommands A list to which any sub-commands generated during execution are added.
     * @return true if the resource was successfully dropped off; false otherwise.
     */
    bool onExecute(int deltaTimeMs, int currentTick, std::list<Command*>& subCommands) override
    {
        findClosestDropOffBuilding();

        if (isDropOffCloseEnough())
        {
            dropResource();
            return true;
        }
        else
        {
            goToDropOffBuilding(subCommands);
        }
        return false;
    }

    /**
     * Updates the animation state for the entity by setting its action to the carrying action
     * for the specified resource type, resets the animation frame, and marks the entity as dirty.
     * This ensures the entity's animation reflects the current resource being carried.
     */
    void animate()
    {
        auto [action, animation] = m_stateMan->getComponents<CompAction, CompAnimation>(m_entityID);

        action.action = m_gatherer->getCarryingAction(resourceType);
        animation.frame = 0;
        StateManager::markDirty(m_entityID);
    }

    /**
     * @brief Adds a movement command to subCommands to move the gatherer to the drop-off building.
     *
     * This function creates a CmdMove command, sets its target entity to the drop-off building,
     * configures its action and priority, and appends it to the provided subCommands list.
     *
     * @param subCommands A list of Command pointers to which the movement command will be added.
     */
    void goToDropOffBuilding(std::list<Command*>& subCommands)
    {
        if (m_dropOffEntity != entt::null)
        {
            const auto& dropOff = m_stateMan->getComponent<CompTransform>(m_dropOffEntity);

            spdlog::debug("Target {} at {} is not close enough to drop-off, moving...",
                          m_dropOffEntity, dropOff.position.toString());
            auto moveCmd = ObjectPool<CmdMove>::acquire();
            moveCmd->targetEntity = m_dropOffEntity;
            moveCmd->actionOverride = m_gatherer->getCarryingAction(resourceType);
            moveCmd->setPriority(getPriority() + CHILD_PRIORITY_OFFSET);
            subCommands.push_back(moveCmd);
        }
    }

    /**
     * @brief Checks if the drop-off entity is close enough to the current entity.
     *
     * This function determines whether the unit is within an acceptable proximity
     * to the designated drop-off building.
     * It verifies that the drop-off entity is valid and checks if the unit's position
     * and radius overlap with the building's land rectangle.
     *
     * @return true if the drop-off entity is close enough; false otherwise.
     */
    bool isDropOffCloseEnough() const
    {
        if (m_dropOffEntity != entt::null)
        {
            auto& building = m_stateMan->getComponent<CompBuilding>(m_dropOffEntity);
            auto rect = building.getLandInFeetRect();

            auto unitPos = m_components->transform.position;
            auto unitRadiusSq = m_components->transform.goalRadiusSquared;

            return overlaps(unitPos, unitRadiusSq, rect);
        }
        return false;
    }

    /**
     * @brief Checks if the drop-off entity exists and is valid for resource drop-off.
     *
     * This function verifies that the drop-off building is valid, has not been destroyed (since the
     * target was set), and is still owned by the current player (i.e. not converted). Returns true
     * if all conditions are met, otherwise false.
     *
     * @return true if the drop-off entity is found and valid; false otherwise.
     */
    bool isDropOffFoundAndValid() const
    {
        if (m_dropOffEntity != entt::null)
        {
            const auto& info = m_stateMan->getComponent<CompEntityInfo>(m_dropOffEntity);
            return !info.isDestroyed &&
                   m_components->player.player->isBuildingOwned(m_dropOffEntity);
        }
        return false;
    }

    /**
     * @brief Finds the closest valid drop-off building for the current unit and updates
     * m_dropOffEntity.
     *
     * This function searches through all buildings owned by the player to find the nearest building
     * that can accept the specified resource type as a drop-off point. It calculates the squared
     * distance between the unit and each candidate building, selecting the one with the smallest
     * distance.
     *
     * @note If a valid drop-off building is already found and valid, the function returns early.
     * @note Not finding a drop-off building is considered a valid scenario. So the unit would stand
     * still.
     */
    void findClosestDropOffBuilding()
    {
        if (isDropOffFoundAndValid())
            return;

        m_dropOffEntity = entt::null;
        float currentClosestDistance = std::numeric_limits<float>::max();

        for (auto buildingEntity : m_components->player.player->getMyBuildings())
        {
            auto [transform, building] =
                m_stateMan->getComponents<CompTransform, CompBuilding>(buildingEntity);

            if (building.acceptResource(resourceType))
            {
                auto distance =
                    transform.position.distanceSquared(m_components->transform.position);

                if (distance < currentClosestDistance)
                {
                    currentClosestDistance = distance;
                    m_dropOffEntity = buildingEntity;
                }
            }
        }

        if (m_dropOffEntity != entt::null)
        {
            spdlog::debug("Selected {} at distance sq {} as the drop off building", m_dropOffEntity,
                          currentClosestDistance);
        }
        // Not being able to find a drop-off point is a valid scenario
    }

    /**
     * @brief Drops the gathered resource and grants it to the player.
     *
     * This function adds the gathered amount of the specified resource type to the
     * player's resources, and resets the gatherer's gathered amount to zero so the unit
     * can continue to gather.
     */
    void dropResource()
    {
        spdlog::debug("Resource {} of type {} is dropping off...", m_gatherer->gatheredAmount,
                      resourceType);
        m_components->player.player->grantResource(resourceType, m_gatherer->gatheredAmount);
        m_gatherer->gatheredAmount = 0;
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
    static bool overlaps(const Feet& unitPos, float radiusSq, const Rect<float>& buildingRect)
    {
        float xMin = buildingRect.x;
        float xMax = buildingRect.x + buildingRect.w;
        float yMin = buildingRect.y;
        float yMax = buildingRect.y + buildingRect.h;

        float closestX = std::clamp(unitPos.x, xMin, xMax);
        float closestY = std::clamp(unitPos.y, yMin, yMax);

        float dx = unitPos.x - closestX;
        float dy = unitPos.y - closestY;

        return (dx * dx + dy * dy) <= radiusSq;
    }
};

} // namespace core

#endif