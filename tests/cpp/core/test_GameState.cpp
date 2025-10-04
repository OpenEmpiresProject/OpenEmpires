#ifndef TEST_GAMESTATE_H
#define TEST_GAMESTATE_H

#include <gtest/gtest.h>
#include "StateManager.h"
#include "ServiceRegistry.h"


namespace core
{
    TEST(GameStateTest, CreateEntity)
    {
        auto stateMan = new StateManager();

        uint32_t entity = stateMan->createEntity();
        EXPECT_TRUE(stateMan->isEntityValid(entity)) << "Entity should be valid after creation";
    }

    TEST(GameStateTest, DestroyEntity)
    {
        auto stateMan = new StateManager();

        uint32_t entity = stateMan->createEntity();
        stateMan->destroyEntity(entity);
        EXPECT_FALSE(stateMan->isEntityValid(entity)) << "Entity should be invalid after destruction";
    }

    TEST(GameStateTest, IsEntityValid)
    {
        auto stateMan = new StateManager();

        uint32_t entity = stateMan->createEntity();
        EXPECT_TRUE(stateMan->isEntityValid(entity)) << "Entity should be valid after creation";

        stateMan->destroyEntity(entity);
        EXPECT_FALSE(stateMan->isEntityValid(entity)) << "Entity should be invalid after destruction";
    }

    TEST(GameStateTest, AddComponent)
    {
        auto stateMan = new StateManager();

        uint32_t entity = stateMan->createEntity();
        struct TestComponent { int value; };
        stateMan->addComponent<TestComponent>(entity, 42);

        EXPECT_TRUE(stateMan->hasComponent<TestComponent>(entity)) << "Entity should have TestComponent";
        EXPECT_EQ(stateMan->getComponent<TestComponent>(entity).value, 42) << "TestComponent value should be 42";
    }

    TEST(GameStateTest, HasComponent)
    {
        auto stateMan = new StateManager();
        uint32_t entity = stateMan->createEntity();
        struct TestComponent { int value; };
        EXPECT_FALSE(stateMan->hasComponent<TestComponent>(entity)) << "Entity should not have TestComponent initially";

        stateMan->addComponent<TestComponent>(entity, 42);
        EXPECT_TRUE(stateMan->hasComponent<TestComponent>(entity)) << "Entity should have TestComponent after adding it";
    }

    TEST(GameStateTest, GetComponent)
    {
        auto stateMan = new StateManager();

        uint32_t entity = stateMan->createEntity();
        struct TestComponent { int value; };
        stateMan->addComponent<TestComponent>(entity, 42);

        TestComponent& component = stateMan->getComponent<TestComponent>(entity);
        EXPECT_EQ(component.value, 42) << "TestComponent value should be 42";
    }

    TEST(GameStateTest, GetComponents)
    {
        auto stateMan = new StateManager();

        uint32_t entity = stateMan->createEntity();
        struct TestComponent { int value; };
        struct AnotherComponent { float data; };

        stateMan->addComponent<TestComponent>(entity, 42);
        stateMan->addComponent<AnotherComponent>(entity, 3.14f);

        auto [testComp, anotherComp] = stateMan->getComponents<TestComponent, AnotherComponent>(entity);
        EXPECT_EQ(testComp.value, 42) << "TestComponent value should be 42";
        EXPECT_FLOAT_EQ(anotherComp.data, 3.14f) << "AnotherComponent data should be 3.14";
    }

    TEST(GameStateTest, ClearAll)
    {
        auto stateMan = new StateManager();

        uint32_t entity1 = stateMan->createEntity();
        uint32_t entity2 = stateMan->createEntity();

        stateMan->clearAll();

        EXPECT_FALSE(stateMan->isEntityValid(entity1)) << "Entity1 should be invalid after clearing";
        EXPECT_FALSE(stateMan->isEntityValid(entity2)) << "Entity2 should be invalid after clearing";
    }
}

#endif