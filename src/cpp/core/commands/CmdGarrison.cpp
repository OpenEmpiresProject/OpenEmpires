#include "CmdGarrison.h"

#include "../components/CompGarrison.h"
#include "Feet.h"
#include "Rect.h"
#include "StateManager.h"
#include "commands/CmdMove.h"
#include "components/CompBuilding.h"
#include "components/CompEntityInfo.h"
#include "components/CompTransform.h"
#include "components/CompUnit.h"
#include "debug.h"
#include "utils/Logger.h"
#include "utils/ObjectPool.h"

using namespace core;

void CmdGarrison::onStart()
{
}

void CmdGarrison::onQueue()
{
}

bool CmdGarrison::onExecute(int deltaTimeMs, int currentTick, std::list<Command*>& subCommands)
{
    if (isCloseEnough())
    {
        garrison();
    }
    else
    {
        moveCloser(subCommands);
    }
    return isComplete();
}

std::string CmdGarrison::toString() const
{
    return "garrison";
}

void CmdGarrison::destroy()
{
    ObjectPool<CmdGarrison>::release(this);
}

bool CmdGarrison::isCloseEnough()
{
    debug_assert(target != entt::null, "Proposed entity to build is null");

    auto& building = m_stateMan->getComponent<CompBuilding>(target);
    auto rect = building.getLandInFeetRect();

    auto unitPos = m_components->transform.position;
    auto unitRadiusSq = m_components->transform.goalRadiusSquared;

    return overlaps(unitPos, unitRadiusSq, rect);
}

bool CmdGarrison::isComplete()
{
    return m_components->unit.isGarrisoned;
}

void CmdGarrison::moveCloser(std::list<Command*>& subCommands)
{
    debug_assert(target != entt::null, "Proposed entity to build is null");

    const auto& targetPosition = m_stateMan->getComponent<CompTransform>(target).position;

    spdlog::debug("Target {} at {} is not close enough to build, moving...", target,
                  targetPosition.toString());
    auto moveCmd = ObjectPool<CmdMove>::acquire();
    moveCmd->targetEntity = target;
    moveCmd->setPriority(getPriority() + CHILD_PRIORITY_OFFSET);
    subCommands.push_back(moveCmd);
}

bool CmdGarrison::overlaps(const Feet& unitPos, float radiusSq, const Rect<float>& buildingRect)
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

void CmdGarrison::garrison()
{
    auto& garrisonBuilding = m_stateMan->getComponent<CompGarrison>(target);
    CompGarrison::GarrisonedUnit garrisonedUnit(m_components->entityInfo.entityType, m_entityID);
    garrisonBuilding.garrisonedUnits.push_back(garrisonedUnit);
    m_components->unit.isGarrisoned = true;

    m_stateMan->gameMap().removeEntity(MapLayerType::UNITS,
                                       m_components->transform.getTilePosition(), m_entityID);
}
