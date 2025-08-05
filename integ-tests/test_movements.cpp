#ifndef TEST_EVENT_H
#define TEST_EVENT_H

#include "IntegTestBase.h"

using namespace core;
using namespace game;

class MovementsTest : public IntegTestBase
{
public:
    void SetUp() override 
    {
        IntegTestBase::SetUp();
        auto player = m_api->getPrimaryPlayer();
        villager = m_api->createVillager(player, Feet(5000, 5000));
    }
    void TearDown() override 
    {
        IntegTestBase::TearDown();

        if (villager != entt::null)
            m_api->deleteEntity(villager);
    }

    uint32_t villager = entt::null;

};

TEST_F(MovementsTest, BeingIdleAtCreation) 
{
    ASSERT_WAIT_FOR(m_api->getCurrentAction(villager) == UnitAction::IDLE, 1000);
}

TEST_F(MovementsTest, VillagerWalk) 
{
    m_api->commandToMove(villager, Feet(6000, 5000));

    ASSERT_WAIT_FOR(m_api->getCurrentAction(villager) == UnitAction::MOVE, 1000);
    ASSERT_WAIT_FOR((m_api->getUnitPosition(villager) - Feet(6000, 5000)).length() < 100, 5000);
}

TEST_F(MovementsTest, VillagerWalkAllDirections) 
{
    // Screen South-East
    m_api->commandToMove(villager, Feet(6000, 5000));
    ASSERT_WAIT_FOR(m_api->getCurrentAction(villager) == UnitAction::MOVE, 1000);
    ASSERT_WAIT_FOR(m_api->getCurrentAction(villager) == UnitAction::IDLE, 5000);

    // Screen West
    m_api->commandToMove(villager, Feet(5000, 6000));
    ASSERT_WAIT_FOR(m_api->getCurrentAction(villager) == UnitAction::MOVE, 1000);
    ASSERT_WAIT_FOR(m_api->getCurrentAction(villager) == UnitAction::IDLE, 7000);

    // Screen East
    m_api->commandToMove(villager, Feet(6000, 5000));
    ASSERT_WAIT_FOR(m_api->getCurrentAction(villager) == UnitAction::MOVE, 1000);
    ASSERT_WAIT_FOR(m_api->getCurrentAction(villager) == UnitAction::IDLE, 7000);

    // Screen South-West
    m_api->commandToMove(villager, Feet(6000, 6000));
    ASSERT_WAIT_FOR(m_api->getCurrentAction(villager) == UnitAction::MOVE, 1000);
    ASSERT_WAIT_FOR(m_api->getCurrentAction(villager) == UnitAction::IDLE, 5000);

    // Screen North-West
    m_api->commandToMove(villager, Feet(5000, 6000));
    ASSERT_WAIT_FOR(m_api->getCurrentAction(villager) == UnitAction::MOVE, 1000);
    ASSERT_WAIT_FOR(m_api->getCurrentAction(villager) == UnitAction::IDLE, 5000);

    // Screen North-East
    m_api->commandToMove(villager, Feet(5000, 5000));
    ASSERT_WAIT_FOR(m_api->getCurrentAction(villager) == UnitAction::MOVE, 1000);
    ASSERT_WAIT_FOR(m_api->getCurrentAction(villager) == UnitAction::IDLE, 5000);

    // Screen South
    m_api->commandToMove(villager, Feet(6000, 6000));
    ASSERT_WAIT_FOR(m_api->getCurrentAction(villager) == UnitAction::MOVE, 1000);
    ASSERT_WAIT_FOR(m_api->getCurrentAction(villager) == UnitAction::IDLE, 7000);

    // Screen North
    m_api->commandToMove(villager, Feet(5000, 5000));
    ASSERT_WAIT_FOR(m_api->getCurrentAction(villager) == UnitAction::MOVE, 1000);
    ASSERT_WAIT_FOR(m_api->getCurrentAction(villager) == UnitAction::IDLE, 7000);
}

#endif
