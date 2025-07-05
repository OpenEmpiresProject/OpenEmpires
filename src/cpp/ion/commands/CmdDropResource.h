#ifndef CMDDROPRESOURCE_H
#define CMDDROPRESOURCE_H

#include "Coordinates.h"
#include "Feet.h"
#include "GameState.h"
#include "Player.h"
#include "ServiceRegistry.h"
#include "commands/CmdMove.h"
#include "commands/Command.h"
#include "components/CompBuilding.h"
#include "components/CompEntityInfo.h"
#include "components/CompPlayer.h"
#include "components/CompResourceGatherer.h"
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
        auto& playerComp = Entity::getComponent<CompPlayer>(m_entityID);
        player = playerComp.player;
        gatherer = &Entity::getComponent<CompResourceGatherer>(m_entityID);
    }

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

    void animate()
    {
        auto [action, animation, dirty] =
            GameState::getInstance().getComponents<CompAction, CompAnimation, CompDirty>(
                m_entityID);

        action.action = gatherer->getCarryingAction(resourceType);
        auto& actionAnimation = animation.animations[action.action];
        animation.frame = 0;
        dirty.markDirty(m_entityID);
    }

    void goToDropOffBuilding(std::list<Command*>& subCommands)
    {
        if (dropOffEntity != entt::null)
        {
            auto& dropOff = Entity::getComponent<CompTransform>(dropOffEntity);

            spdlog::debug("Target {} at {} is not close enough to drop-off, moving...",
                          dropOffEntity, dropOff.position.toString());
            auto move = ObjectPool<CmdMove>::acquire();
            move->targetEntity = dropOffEntity;
            move->actionOverride = gatherer->getCarryingAction(resourceType);
            move->setPriority(getPriority() + CHILD_PRIORITY_OFFSET);
            subCommands.push_back(move);
        }
    }

    bool isDropOffCloseEnough()
    {
        if (dropOffEntity == entt::null)
            return false;

        auto& transformMy = Entity::getComponent<CompTransform>(m_entityID);
        auto [transform, building] =
            Entity::getComponents<CompTransform, CompBuilding>(dropOffEntity);
        auto pos = transformMy.position;
        auto radiusSq = transformMy.goalRadiusSquared;
        auto rect = building.getLandInFeetRect(transform.position);

        spdlog::debug("building rect {}, unit pos {}, unit radiusSq {}", rect.toString(),
                      pos.toString(), radiusSq);

        return overlaps(pos, radiusSq, rect);
    }

    bool isDropOffFoundAndValid()
    {
        if (dropOffEntity != entt::null)
        {
            auto& info = Entity::getComponent<CompEntityInfo>(dropOffEntity);
            return !info.isDestroyed && player->isBuildingOwned(dropOffEntity);
        }
        return false;
    }

    void findClosestDropOffBuilding()
    {
        if (isDropOffFoundAndValid())
            return;

        dropOffEntity = entt::null;
        auto& unitTransform = Entity::getComponent<CompTransform>(m_entityID);
        float currentClosestDistance = std::numeric_limits<float>::max();

        for (auto buildingEntity : player->getMyBuildings())
        {
            auto [transform, building] =
                Entity::getComponents<CompTransform, CompBuilding>(buildingEntity);
            if (building.dropOffForResourceType != resourceType)
                continue;

            auto distance = transform.position.distanceSquared(unitTransform.position);

            if (distance < currentClosestDistance)
            {
                currentClosestDistance = distance;
                dropOffEntity = buildingEntity;
            }
        }

        if (dropOffEntity != entt::null)
        {
            spdlog::debug("Selected {} at distance sq {} as the drop off building", dropOffEntity,
                          currentClosestDistance);
        }
        // Not being able to find a drop-off point is a valid scenario
    }

    void dropResource()
    {
        spdlog::debug("Resource {} of type {} is dropping off...", gatherer->gatheredAmount,
                      resourceType);
        player->grantResource(resourceType, gatherer->gatheredAmount);
        gatherer->gatheredAmount = 0;
    }

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