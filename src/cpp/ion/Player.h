#ifndef PLAYER_H
#define PLAYER_H

#include "FogOfWar.h"
#include "Resource.h"

#include <limits>
#include <unordered_set>
#include <vector>

namespace ion
{
class Player
{
  public:
    Player();
    ~Player();

    uint8_t getId() const
    {
        return m_id;
    }

    void init(uint8_t id);
    bool isValid() const
    {
        return m_id != INVALID_ID;
    }
    void grantResource(uint8_t resourceType, uint32_t amount);
    bool spendResource(uint8_t resourceType, uint32_t amount);
    uint32_t getResourceAmount(uint8_t resourceType);
    bool hasResource(uint8_t resourceType, uint32_t amount);

    void addEntity(uint32_t entityId);
    void removeEntity(uint32_t entityId);
    bool isOwned(uint32_t entityId);

    Ref<FogOfWar> getFOW() const
    {
        return m_fow;
    }

    static constexpr uint8_t INVALID_ID = std::numeric_limits<uint8_t>::max();

  private:
    uint8_t m_id = INVALID_ID;
    std::vector<Resource> m_resources;
    std::unordered_set<uint8_t> m_ownedEntities;
    Ref<FogOfWar> m_fow;
};

} // namespace ion

#endif