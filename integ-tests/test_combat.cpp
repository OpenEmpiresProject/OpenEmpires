#include "IntegTestBase.h"

using namespace core;
using namespace game;

class Combat : public IntegTestBase, public ::testing::Test
{
public:
    void SetUp() override
    {
        auto player = m_api->getPrimaryPlayer();
        playerId = player->getId();
        auto militiaEntityType = m_api->getEntityType("militia");
        militia = m_api->createUnit(player, Feet(5000, 5000), militiaEntityType);

        auto player2 = m_api->getPlayer(2);
        villager = m_api->createVillager(player2, Feet(6000, 6000));
    }

    void TearDown() override
    {
        if (villager != entt::null)
            m_api->deleteEntity(villager);

        if (militia != entt::null)
            m_api->deleteEntity(militia);
    }

    uint32_t militia = entt::null;
    uint32_t villager = entt::null;
    int playerId = -1;
};
 
TEST_F(Combat, AttackMove_Infantry) 
{
    m_api->commandToAttack(militia, villager);
    ASSERT_WAIT_FOR(m_api->getHealth(villager) <= 0, 10000);
}
