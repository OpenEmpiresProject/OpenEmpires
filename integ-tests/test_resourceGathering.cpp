#include "IntegTestBase.h"

using namespace core;
using namespace game;

class ResourceGathering : public IntegTestBase, public ::testing::Test
{
public:
    void SetUp() override
    {
        auto player = m_api->getPrimaryPlayer();
        playerId = player->getId();
        villager = m_api->createVillager(player, Feet(5000, 5000));
    }

    void TearDown() override
    {
        if (villager != entt::null)
            m_api->deleteEntity(villager);

        for (auto entity : m_api->getConstructionSites(playerId))
            m_api->deleteEntity(entity);

        for (auto entity : m_api->getBuildings(playerId))
            m_api->deleteEntity(entity);

        int attempts = 0;
        while (attempts++ < 10 && (!m_api->getConstructionSites(playerId).empty() ||
                                   !m_api->getBuildings(playerId).empty()))
        {
            sleep(500);
        }
    }

    uint32_t villager = entt::null;
    int playerId = -1;
};

class ResourceGatheringOrientationParams : public ResourceGathering,
                                          public ::testing::WithParamInterface<BuildingOrientation>
                                          
{
};
 
 TEST_F(ResourceGathering, GatherGold) 
 {
     int miningCampEntityType = m_api->getEntityType("mine_camp");
     m_api->placeBuilding(playerId, miningCampEntityType, Tile(23, 23).toFeet());
    ASSERT_WAIT_FOR(m_api->getConstructionSites(playerId).size() == 1, 1000);
    m_api->build(villager, *(m_api->getConstructionSites(playerId).begin()));
    ASSERT_WAIT_FOR(m_api->getConstructionSites(playerId).size() == 0, 10000);

    ASSERT_WAIT_FOR(m_api->getPlayerResourceAmount(playerId, ResourceType::GOLD) >= 100, 20000);
 }

 TEST_F(ResourceGathering, GatherWood)
 {
     int lumberCampEntityType = m_api->getEntityType("wood_camp");
     m_api->placeBuilding(playerId, lumberCampEntityType, Tile(20, 24).toFeet());
     ASSERT_WAIT_FOR(m_api->getConstructionSites(playerId).size() == 1, 1000);
     m_api->build(villager, *(m_api->getConstructionSites(playerId).begin()));
     ASSERT_WAIT_FOR(m_api->getConstructionSites(playerId).size() == 0, 10000);

     ASSERT_WAIT_FOR(m_api->getPlayerResourceAmount(playerId, ResourceType::WOOD) >= 100, 20000);
 }