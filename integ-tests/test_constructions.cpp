#ifndef TEST_CONSTRUCTION
#define TEST_CONSTRUCTION

#include "IntegTestBase.h"

using namespace core;
using namespace game;

class ConstructionTestBase : public IntegTestBase
{
public:
    void CommonSetUp() 
    {
        auto player = m_api->getPrimaryPlayer();
        playerId = player->getId();
        villager = m_api->createVillager(player, Feet(5000, 5000));
    }
    void CommonTearDown() 
    {
        if (villager != entt::null)
            m_api->deleteEntity(villager);

        for (auto entity : m_api->getConstructionSites(playerId))
            m_api->deleteEntity(entity);

        for (auto entity : m_api->getBuildings(playerId))
            m_api->deleteEntity(entity);

        while (not m_api->getConstructionSites(playerId).empty())
        {
            sleep(500);
        }
        while (not m_api->getBuildings(playerId).empty())
        {
            sleep(500);
        }
    }

    uint32_t villager = entt::null;
    int playerId = -1;
    const uint32_t houseEntityType = 12;
    const uint32_t gateEntityType = 17;
};

class ConstructionTest : public ::testing::Test, protected ConstructionTestBase
{
  protected:
    void SetUp() override
    {
        CommonSetUp();
    }
    void TearDown() override
    {
        CommonTearDown();
    }
};

class ConstructionTestOrientationParams : public ::testing::TestWithParam<BuildingOrientation>,
                               protected ConstructionTestBase
{
  protected:
    void SetUp() override
    {
        CommonSetUp();
    }
    void TearDown() override
    {
        CommonTearDown();
    }
};

TEST_F(ConstructionTest, Placement) 
{
    m_api->placeBuilding(playerId, houseEntityType, Feet(6000, 6000));

    ASSERT_WAIT_FOR(m_api->getConstructionSites(playerId).size() == 1, 1000);
}

TEST_F(ConstructionTest, PlaceAndBuildHouse)
{
    m_api->placeBuilding(playerId, houseEntityType, Feet(5500, 5500));

    ASSERT_WAIT_FOR(m_api->getConstructionSites(playerId).size() == 1, 1000);

    m_api->build(villager, *(m_api->getConstructionSites(playerId).begin()));

    ASSERT_WAIT_FOR(m_api->getConstructionSites(playerId).size() == 0, 10000);
    ASSERT_WAIT_FOR(m_api->getBuildings(playerId).size() == 1, 1000);

    sleep(500); // Just to give a glimpse of the building 
}


TEST_P(ConstructionTestOrientationParams, PlaceAndBuildGate)
{
    const auto& ori = GetParam();

    m_api->placeBuilding(playerId, gateEntityType, Feet(5500, 5500), ori);

    ASSERT_WAIT_FOR(m_api->getConstructionSites(playerId).size() == 1, 1000);

    m_api->build(villager, *(m_api->getConstructionSites(playerId).begin()));

    ASSERT_WAIT_FOR(m_api->getConstructionSites(playerId).size() == 0, 10000);
    ASSERT_WAIT_FOR(m_api->getBuildings(playerId).size() == 1, 1000);

    sleep(500); // Just to give a glimpse of the building
}

INSTANTIATE_TEST_SUITE_P(Default,
                         ConstructionTestOrientationParams,
                         ::testing::Values(BuildingOrientation::DIAGONAL_FORWARD,
                                           BuildingOrientation::DIAGONAL_BACKWARD,
                                           BuildingOrientation::HORIZONTAL,
                                           BuildingOrientation::VERTICAL));

#endif