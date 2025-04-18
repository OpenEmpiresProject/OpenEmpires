#ifndef TEST_GAMESTATE_H
#define TEST_GAMESTATE_H

#include <gtest/gtest.h>
#include "GameState.h"

namespace aion
{
    TEST(GameStateTest, CreateEntity)
    {
        GameState gameState;

        entt::entity entity = gameState.createEntity();
        EXPECT_TRUE(gameState.isEntityValid(entity)) << "Entity should be valid after creation";
    }

    TEST(GameStateTest, DestroyEntity)
    {
        GameState gameState;

        entt::entity entity = gameState.createEntity();
        gameState.destroyEntity(entity);
        EXPECT_FALSE(gameState.isEntityValid(entity)) << "Entity should be invalid after destruction";
    }

    TEST(GameStateTest, IsEntityValid)
    {
        GameState gameState;

        entt::entity entity = gameState.createEntity();
        EXPECT_TRUE(gameState.isEntityValid(entity)) << "Entity should be valid after creation";

        gameState.destroyEntity(entity);
        EXPECT_FALSE(gameState.isEntityValid(entity)) << "Entity should be invalid after destruction";
    }

    TEST(GameStateTest, AddComponent)
    {
        GameState gameState;

        entt::entity entity = gameState.createEntity();
        struct TestComponent { int value; };
        gameState.addComponent<TestComponent>(entity, 42);

        EXPECT_TRUE(gameState.hasComponent<TestComponent>(entity)) << "Entity should have TestComponent";
        EXPECT_EQ(gameState.getComponent<TestComponent>(entity).value, 42) << "TestComponent value should be 42";
    }

    TEST(GameStateTest, HasComponent)
    {
        GameState gameState;

        entt::entity entity = gameState.createEntity();
        struct TestComponent { int value; };
        EXPECT_FALSE(gameState.hasComponent<TestComponent>(entity)) << "Entity should not have TestComponent initially";

        gameState.addComponent<TestComponent>(entity, 42);
        EXPECT_TRUE(gameState.hasComponent<TestComponent>(entity)) << "Entity should have TestComponent after adding it";
    }

    TEST(GameStateTest, GetComponent)
    {
        GameState gameState;

        entt::entity entity = gameState.createEntity();
        struct TestComponent { int value; };
        gameState.addComponent<TestComponent>(entity, 42);

        TestComponent& component = gameState.getComponent<TestComponent>(entity);
        EXPECT_EQ(component.value, 42) << "TestComponent value should be 42";
    }

    TEST(GameStateTest, GetComponents)
    {
        GameState gameState;

        entt::entity entity = gameState.createEntity();
        struct TestComponent { int value; };
        struct AnotherComponent { float data; };

        gameState.addComponent<TestComponent>(entity, 42);
        gameState.addComponent<AnotherComponent>(entity, 3.14f);

        auto [testComp, anotherComp] = gameState.getComponents<TestComponent, AnotherComponent>(entity);
        EXPECT_EQ(testComp.value, 42) << "TestComponent value should be 42";
        EXPECT_FLOAT_EQ(anotherComp.data, 3.14f) << "AnotherComponent data should be 3.14";
    }

    TEST(GameStateTest, ClearAll)
    {
        GameState gameState;

        entt::entity entity1 = gameState.createEntity();
        entt::entity entity2 = gameState.createEntity();

        gameState.clearAll();

        EXPECT_FALSE(gameState.isEntityValid(entity1)) << "Entity1 should be invalid after clearing";
        EXPECT_FALSE(gameState.isEntityValid(entity2)) << "Entity2 should be invalid after clearing";
    }
}

#endif