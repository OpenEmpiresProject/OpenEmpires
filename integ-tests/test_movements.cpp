#ifndef TEST_EVENT_H
#define TEST_EVENT_H

#include "IntegTestBase.h"

using namespace ion;
using namespace game;

class MovementsTest : public IntegTestBase
{
};

TEST_F(MovementsTest, BeingIdleAtCreation) 
{
    auto player = m_api->getPrimaryPlayer();
    auto villager = m_api->createVillager(player, Feet(5000, 5000));
    auto villagers = m_api->getVillagers();

    ASSERT_EQ(villagers.size(), 1);

    sleep(1000);

    // Villager should be still idle after 1s
    ASSERT_EQ(m_api->getCurrentAction(villager), UnitAction::IDLE);
}

#endif
