#include "SubSystemRegistry.h"
#include "gtest/gtest.h"
#include <memory>

namespace core
{

class MockComponent : public SubSystem
{
  public:
    // Override SubSystem's methods with mock implementations
    MockComponent() : SubSystem((std::stop_token*) nullptr)
    {
    }
    void init()
    {
        initCount++;
    }
    void shutdown()
    {
        shutdownCount++;
    }

    int initCount = 0;
    int shutdownCount = 0;
};

TEST(ComponentRegistryTest, RegisterAndRetrieveComponent)
{
    SubSystemRegistry& registry = SubSystemRegistry::getInstance();
    auto mockComponent = std::make_unique<MockComponent>();
    MockComponent* mockComponentPtr = mockComponent.get();

    registry.registerSubSystem("TestComponent", std::move(mockComponent));
    auto retrievedComponent = registry.getSubSystem("TestComponent");

    EXPECT_EQ(retrievedComponent.get(), mockComponentPtr);
}

TEST(ComponentRegistryTest, RetrieveNonExistentComponent)
{
    SubSystemRegistry& registry = SubSystemRegistry::getInstance();
    auto retrievedComponent = registry.getSubSystem("NonExistentComponent");

    EXPECT_EQ(retrievedComponent, nullptr);
}

TEST(ComponentRegistryTest, InitAllComponents)
{
    SubSystemRegistry& registry = SubSystemRegistry::getInstance();
    auto mockComponent = std::make_unique<MockComponent>();
    MockComponent* mockComponentPtr = mockComponent.get();

    registry.registerSubSystem("TestComponent", std::move(mockComponent));
    registry.initAll();

    ASSERT_EQ(mockComponentPtr->initCount, 1);
}

TEST(ComponentRegistryTest, ShutdownAllComponents)
{
    SubSystemRegistry& registry = SubSystemRegistry::getInstance();
    auto mockComponent = std::make_unique<MockComponent>();
    MockComponent* mockComponentPtr = mockComponent.get();

    registry.registerSubSystem("TestComponent", std::move(mockComponent));
    registry.shutdownAll();

    ASSERT_EQ(mockComponentPtr->shutdownCount, 1);
}
} // namespace core