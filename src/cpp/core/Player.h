#ifndef PLAYER_H
#define PLAYER_H

#include "FogOfWar.h"
#include "GameState.h"
#include "InGameResource.h"

#include <limits>
#include <unordered_set>
#include <vector>

namespace core
{
class Player
{
  public:
    void init(uint8_t id);

    uint8_t getId() const
    {
        return m_id;
    }

    bool isValid() const
    {
        return m_id != INVALID_ID;
    }
    void grantResource(uint8_t resourceType, uint32_t amount);
    bool spendResource(uint8_t resourceType, uint32_t amount);
    uint32_t getResourceAmount(uint8_t resourceType) const;
    bool hasResource(uint8_t resourceType, uint32_t amount) const;

    void addEntity(uint32_t entityId);
    void removeEntity(uint32_t entityId);
    bool isOwned(uint32_t entityId) const;

    uint32_t getHousingCapacity() const
    {
        return m_housingCapacity;
    }

    uint32_t getPopulation() const
    {
        return m_currentPopulation;
    }

    Ref<FogOfWar> getFogOfWar() const
    {
        return m_fow;
    }

    const std::unordered_set<uint32_t>& getMyBuildings() const
    {
        return m_myBuildings;
    }

    bool isBuildingOwned(uint32_t buildingId) const
    {
        return m_myBuildings.contains(buildingId);
    }

  private:
    static constexpr uint8_t INVALID_ID = std::numeric_limits<uint8_t>::max();

    uint8_t m_id = INVALID_ID;
    std::vector<InGameResource> m_resources;
    std::unordered_set<uint8_t> m_ownedEntities;
    Ref<FogOfWar> m_fow;
    std::unordered_set<uint32_t> m_myBuildings;
    Ref<GameState> m_gameState;
    uint32_t m_housingCapacity = 0;
    uint32_t m_currentPopulation = 0;
};

} // namespace core

#endif