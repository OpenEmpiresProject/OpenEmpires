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

void CmdBuild::animate(int deltaTimeMs)
{
    m_components->action.action = UnitAction::BUILDING;
    auto& actionAnimation = m_components->animation.animations[m_components->action.action];

    auto ticksPerFrame = m_settings->getTicksPerSecond() / actionAnimation.speed;
    if (s_totalTicks % ticksPerFrame == 0)
    {
        m_components->dirty.markDirty(m_entityID);
        m_components->animation.frame++;
        m_components->animation.frame %= actionAnimation.frames; // Building is repeatable
    }
}

bool CmdBuild::isCloseEnough()
{
    debug_assert(target != entt::null, "Proposed entity to build is null");

    auto [transform, building] = m_gameState->getComponents<CompTransform, CompBuilding>(target);
    auto pos = m_components->transform.position;
    auto radiusSq = m_components->transform.goalRadiusSquared;
    auto rect = building.getLandInFeetRect(transform.position);

    return overlaps(pos, radiusSq, rect);
}

bool CmdBuild::isComplete()
{
    return m_gameState->getComponent<CompBuilding>(target).constructionProgress == 100;
}

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
