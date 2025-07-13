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

    ASSERT_WAIT_FOR(m_api->getCurrentAction(villager) == UnitAction::IDLE, 1000);
}

TEST_F(MovementsTest, VillagerWalk) 
{
    auto player = m_api->getPrimaryPlayer();
    auto villager = m_api->createVillager(player, Feet(5000, 5000));
    m_api->commandToMove(villager, Feet(6000, 5000));

    ASSERT_WAIT_FOR(m_api->getCurrentAction(villager) == UnitAction::MOVE, 1000);
    ASSERT_WAIT_FOR((m_api->getUnitPosition(villager) - Feet(6000, 5000)).length() < 100, 5000);
}

#endif
