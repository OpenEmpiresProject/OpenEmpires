#include "Player.h"

#include "ServiceRegistry.h"
#include "Settings.h"
#include "components/CompBuilding.h"
#include "components/CompHousing.h"
#include "components/CompUnit.h"
#include "debug.h"

using namespace core;

void Player::init(uint8_t id)
{
    m_id = id;
    for (size_t i = 0; i < Constants::MAX_RESOURCE_TYPES; i++)
    {
        m_resources.push_back(InGameResource(i, 0));
    }
    m_fow = CreateRef<FogOfWar>();
    auto settings = ServiceRegistry::getInstance().getService<Settings>();
    m_fow->init(settings->getWorldSizeInTiles().width, settings->getWorldSizeInTiles().height,
                settings->getFOWRevealStatus());

    m_stateMan = ServiceRegistry::getInstance().getService<StateManager>();
}

void Player::grantResource(uint8_t resourceType, uint32_t amount)
{
    debug_assert(resourceType < Constants::MAX_RESOURCE_TYPES, "Invalid resource type {}",
                 resourceType);

    m_resources[resourceType].amount += amount;
}

bool Player::spendResource(uint8_t resourceType, uint32_t amount)
{
    debug_assert(resourceType < Constants::MAX_RESOURCE_TYPES, "Invalid resource type {}",
                 resourceType);

    if (m_resources[resourceType].amount >= amount)
    {
        m_resources[resourceType].amount -= amount;
        return true;
    }
    return false;
}

uint32_t Player::getResourceAmount(uint8_t resourceType) const
{
    debug_assert(resourceType < Constants::MAX_RESOURCE_TYPES, "Invalid resource type {}",
                 resourceType);

    return m_resources[resourceType].amount;
}

bool Player::hasResource(uint8_t resourceType, uint32_t amount) const
{
    debug_assert(resourceType < Constants::MAX_RESOURCE_TYPES, "Invalid resource type {}",
                 resourceType);

    return m_resources[resourceType].amount >= amount;
}

void Player::addEntity(uint32_t entityId)
{
    m_ownedEntities.insert(entityId);

    if (m_stateMan->hasComponent<CompBuilding>(entityId))
    {
        m_myBuildings.insert(entityId);
        if (m_stateMan->hasComponent<CompHousing>(entityId))
        {
            auto& housing = m_stateMan->getComponent<CompHousing>(entityId);
            m_housingCapacity += housing.housingCapacity;
        }
    }

    if (m_stateMan->hasComponent<CompUnit>(entityId))
    {
        auto& unit = m_stateMan->getComponent<CompUnit>(entityId);
        m_currentPopulation += unit.housingNeed;
    }
}

void Player::removeEntity(uint32_t entityId)
{
    if (isOwned(entityId))
    {
        m_ownedEntities.erase(entityId);

        if (m_stateMan->hasComponent<CompBuilding>(entityId))
        {
            m_myBuildings.erase(entityId);
            if (m_stateMan->hasComponent<CompHousing>(entityId))
            {
                auto& housing = m_stateMan->getComponent<CompHousing>(entityId);
                m_housingCapacity -= housing.housingCapacity;
            }
        }

        if (m_stateMan->hasComponent<CompUnit>(entityId))
        {
            auto& unit = m_stateMan->getComponent<CompUnit>(entityId);
            m_currentPopulation -= unit.housingNeed;
        }
    }
}

bool Player::isOwned(uint32_t entityId) const
{
    return m_ownedEntities.find(entityId) != m_ownedEntities.end();
}