#include "CmdGatherResource.h"

uint32_t core::CmdGatherResource::findClosestResource(uint8_t resourceType,
                                                      const Tile& startTile,
                                                      int maxRadius,
                                                      Ref<StateManager> stateMan)
{
    std::queue<std::pair<Tile, int>> toVisit; // Pair: position, current distance
    std::unordered_set<Tile> visited;

    uint32_t resourceEntity = entt::null;

    toVisit.push({startTile, 0});
    visited.insert(startTile);

    static const std::vector<Tile> directions = {{0, -1},  {1, 0},  {0, 1}, {-1, 0},
                                                 {-1, -1}, {1, -1}, {1, 1}, {-1, 1}};

    while (!toVisit.empty())
    {
        auto [current, distance] = toVisit.front();
        toVisit.pop();

        if (doesTileHaveResource(resourceType, current, resourceEntity, stateMan))
        {
            return resourceEntity;
        }

        if (distance >= maxRadius)
        {
            continue; // Stop expanding beyond max radius
        }

        for (const auto& dir : directions)
        {
            Tile neighbor{current.x + dir.x, current.y + dir.y};

            if (!visited.contains(neighbor))
            {
                visited.insert(neighbor);
                toVisit.push({neighbor, distance + 1});
            }
        }
    }
    return entt::null; // Not found within radius
}

bool core::CmdGatherResource::doesTileHaveResource(uint8_t resourceType,
                                                   const Tile& posTile,
                                                   uint32_t& resourceEntity,
                                                   Ref<StateManager> stateMan)
{
    resourceEntity = entt::null;
    auto& map = stateMan->gameMap();
    auto e = map.getEntity(MapLayerType::STATIC, posTile);
    if (e != entt::null)
    {
        const auto& info = stateMan->getComponent<CompEntityInfo>(e);
        if (!info.isDestroyed)
        {
            if (stateMan->hasComponent<CompResource>(e))
            {
                const auto& resource = stateMan->getComponent<CompResource>(e);
                if (resource.original.value().type == resourceType)
                {
                    resourceEntity = e;
                    return true;
                }
            }
        }
    }
    return false;
}

void core::CmdGatherResource::onStart()
{
    spdlog::debug("Start gathering resource...");
}

void core::CmdGatherResource::onQueue()
{
    // TODO: Reset frame to zero (since this is a new command)
    setTarget(target);

    m_collectedResourceAmount = 0;
    m_gatherer = &(m_stateMan->getComponent<CompResourceGatherer>(m_entityID));
}

void core::CmdGatherResource::setTarget(uint32_t targetEntity)
{
    debug_assert(m_stateMan->hasComponent<CompResource>(targetEntity),
                 "Target entity is not a resource");

    target = targetEntity;

    auto [transformTarget, resourceTarget] =
        m_stateMan->getComponents<CompTransform, CompResource>(target);
    m_targetPosition = transformTarget.position;
    m_targetResourceType = resourceTarget.original.value().type;
}

bool core::CmdGatherResource::onExecute(int deltaTimeMs,
                                        int currentTick,
                                        std::list<Command*>& subCommands)
{
    if (isResourceAvailable() == false)
    {
        if (lookForAnotherSimilarResource() == false)
        {
            // Could not find another resource, nothing to gather, done with the command
            return true;
        }
    }

    if (isCloseEnough())
    {
        animate(currentTick);
        gather(deltaTimeMs);
    }
    else
    {
        moveCloser(subCommands);
    }

    if (isFull())
    {
        dropOff(subCommands);
    }
    return false;
}

std::string core::CmdGatherResource::toString() const
{
    return "gather-resource";
}

core::Command* core::CmdGatherResource::clone()
{
    return ObjectPool<CmdGatherResource>::acquire(*this);
}

void core::CmdGatherResource::destroy()
{
    ObjectPool<CmdGatherResource>::release(this);
}

void core::CmdGatherResource::moveCloser(std::list<Command*>& subCommands)
{
    spdlog::debug("Target {} at {} is not close enough to gather, moving...", target,
                  m_targetPosition.toString());
    auto moveCmd = ObjectPool<CmdMove>::acquire();
    moveCmd->targetEntity = target;
    moveCmd->setPriority(getPriority() + CHILD_PRIORITY_OFFSET);
    subCommands.push_back(moveCmd);
}

void core::CmdGatherResource::dropOff(std::list<Command*>& subCommands)
{
    spdlog::debug("Unit {} is at full capacity, need to drop off", m_entityID);
    auto dropCmd = ObjectPool<CmdDropResource>::acquire();
    dropCmd->resourceType = m_targetResourceType;
    dropCmd->setPriority(getPriority() + CHILD_PRIORITY_OFFSET);
    subCommands.push_back(dropCmd);
}

bool core::CmdGatherResource::isResourceAvailable() const
{
    if (target != entt::null)
    {
        const auto& info = m_stateMan->getComponent<CompEntityInfo>(target);
        return !info.isDestroyed;
    }
    return false;
}

bool core::CmdGatherResource::isCloseEnough() const
{
    const auto& transformMy = m_components->transform;
    auto threshold = transformMy.goalRadiusSquared;
    auto distanceSquared = transformMy.position.distanceSquared(m_targetPosition);
    return transformMy.position.distanceSquared(m_targetPosition) < (threshold * threshold);
}

void core::CmdGatherResource::animate(int currentTick)
{
    m_components->action.action = m_gatherer->getGatheringAction(m_targetResourceType);
    auto& actionAnimation = m_components->animation.animations[m_components->action.action];

    auto ticksPerFrame = int(m_settings->getTicksPerSecond() /
                             (actionAnimation.value().speed * m_settings->getGameSpeed()));
    if (currentTick % ticksPerFrame == 0)
    {
        StateManager::markDirty(m_entityID);
        m_components->animation.frame++;
        m_components->animation.frame %= actionAnimation.value().frames;
    }
}

bool core::CmdGatherResource::isFull() const
{
    return m_gatherer->gatheredAmount >= m_gatherer->capacity;
}

void core::CmdGatherResource::gather(int deltaTimeMs)
{
    auto& resource = m_stateMan->getComponent<CompResource>(target);

    m_collectedResourceAmount +=
        float(m_choppingSpeed) * m_settings->getGameSpeed() * deltaTimeMs / 1000.0f;
    uint32_t rounded = m_collectedResourceAmount;
    m_collectedResourceAmount -= rounded;

    if (rounded != 0)
    {
        auto actualDelta = std::min(resource.remainingAmount, rounded);
        actualDelta = std::min(actualDelta, (m_gatherer->capacity - m_gatherer->gatheredAmount));
        resource.remainingAmount -= actualDelta;
        m_gatherer->gatheredAmount += actualDelta;
        StateManager::markDirty(target);
    }
}

bool core::CmdGatherResource::lookForAnotherSimilarResource()
{
    spdlog::debug("Looking for another resource...");
    auto tilePos = m_components->transform.position.toTile();

    auto newResource = findClosestResource(
        m_targetResourceType, tilePos, Constants::MAX_RESOURCE_LOOKUP_RADIUS, m_stateMan.getRef());

    if (newResource != entt::null)
    {
        spdlog::debug("Found resource {}", newResource);
        setTarget(newResource);
    }
    else
    {
        spdlog::debug("No resource of type {} found around {}", m_targetResourceType,
                      m_components->transform.position.toString());
    }
    return newResource != entt::null;
}
