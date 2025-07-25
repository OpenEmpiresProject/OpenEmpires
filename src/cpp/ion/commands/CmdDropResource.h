#ifndef CMDDROPRESOURCE_H
#define CMDDROPRESOURCE_H

#include "Coordinates.h"
#include "Feet.h"
#include "GameState.h"
#include "Player.h"
#include "Rect.h"
#include "ServiceRegistry.h"
#include "commands/CmdMove.h"
#include "commands/Command.h"
#include "components/CompAction.h"
#include "components/CompAnimation.h"
#include "components/CompBuilding.h"
#include "components/CompDirty.h"
#include "components/CompEntityInfo.h"
#include "components/CompPlayer.h"
#include "components/CompResourceGatherer.h"
#include "components/CompTransform.h"
#include "debug.h"
#include "utils/ObjectPool.h"

#include <algorithm>

namespace ion
{
class CmdDropResource : public Command
{
  public:
    uint8_t resourceType = Constants::RESOURCE_TYPE_NONE;

  private:
    uint32_t dropOffEntity = entt::null;
    Ref<Player> player;
    CompResourceGatherer* gatherer = nullptr;

    std::string toString() const override
    {
        return "drop-resource";
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
        auto& playerComp = m_gameState->getComponent<CompPlayer>(m_entityID);
        player = playerComp.player;
        gatherer = &(m_gameState->getComponent<CompResourceGatherer>(m_entityID));
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
    bool onExecute(int deltaTimeMs, std::list<Command*>& subCommands) override
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
        auto [action, animation, dirty] =
            m_gameState->getComponents<CompAction, CompAnimation, CompDirty>(m_entityID);

        action.action = gatherer->getCarryingAction(resourceType);
        auto& actionAnimation = animation.animations[action.action];
        animation.frame = 0;
        dirty.markDirty(m_entityID);
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
        if (dropOffEntity != entt::null)
        {
            auto& dropOff = m_gameState->getComponent<CompTransform>(dropOffEntity);

            spdlog::debug("Target {} at {} is not close enough to drop-off, moving...",
                          dropOffEntity, dropOff.position.toString());
            auto move = ObjectPool<CmdMove>::acquire();
            move->targetEntity = dropOffEntity;
            move->actionOverride = gatherer->getCarryingAction(resourceType);
            move->setPriority(getPriority() + CHILD_PRIORITY_OFFSET);
            subCommands.push_back(move);
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
    bool isDropOffCloseEnough()
    {
        if (dropOffEntity == entt::null)
            return false;

        auto& transformMy = m_gameState->getComponent<CompTransform>(m_entityID);
        auto [transform, building] =
            m_gameState->getComponents<CompTransform, CompBuilding>(dropOffEntity);
        auto pos = transformMy.position;
        auto radiusSq = transformMy.goalRadiusSquared;
        auto rect = building.getLandInFeetRect(transform.position);

        spdlog::debug("building rect {}, unit pos {}, unit radiusSq {}", rect.toString(),
                      pos.toString(), radiusSq);

        return overlaps(pos, radiusSq, rect);
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
    bool isDropOffFoundAndValid()
    {
        if (dropOffEntity != entt::null)
        {
            auto& info = m_gameState->getComponent<CompEntityInfo>(dropOffEntity);
            return !info.isDestroyed && player->isBuildingOwned(dropOffEntity);
        }
        return false;
    }

    /**
     * @brief Finds the closest valid drop-off building for the current unit and updates
     * dropOffEntity.
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

        dropOffEntity = entt::null;
        auto& unitTransform = m_gameState->getComponent<CompTransform>(m_entityID);
        float currentClosestDistance = std::numeric_limits<float>::max();

        for (auto buildingEntity : player->getMyBuildings())
        {
            auto [transform, building] =
                m_gameState->getComponents<CompTransform, CompBuilding>(buildingEntity);

            if (building.canDropOff(resourceType))
            {
                auto distance = transform.position.distanceSquared(unitTransform.position);

                if (distance < currentClosestDistance)
                {
                    currentClosestDistance = distance;
                    dropOffEntity = buildingEntity;
                }
            }
        }

        if (dropOffEntity != entt::null)
        {
            spdlog::debug("Selected {} at distance sq {} as the drop off building", dropOffEntity,
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
        spdlog::debug("Resource {} of type {} is dropping off...", gatherer->gatheredAmount,
                      resourceType);
        player->grantResource(resourceType, gatherer->gatheredAmount);
        gatherer->gatheredAmount = 0;
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
    bool overlaps(const Feet& unitPos, float radiusSq, const Rect<float>& buildingRect)
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
};

} // namespace ion

#endif