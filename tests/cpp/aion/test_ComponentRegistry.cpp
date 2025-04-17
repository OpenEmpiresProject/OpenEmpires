#include "ComponentRegistry.h"
#include "gtest/gtest.h"
#include <memory>

using namespace aion;

class MockComponent : public Component {
public:
    // Override Component's methods with mock implementations
    void init() {initCount++;}
    void shutdown() {shutdownCount++;}

    int initCount = 0;
    int shutdownCount = 0;
};

TEST(ComponentRegistryTest, RegisterAndRetrieveComponent) {
    ComponentRegistry& registry = ComponentRegistry::getInstance();
    auto mockComponent = std::make_unique<MockComponent>();
    MockComponent* mockComponentPtr = mockComponent.get();

    registry.registerComponent("TestComponent", std::move(mockComponent));
    Component* retrievedComponent = registry.getComponent("TestComponent");

    EXPECT_EQ(retrievedComponent, mockComponentPtr);
}

TEST(ComponentRegistryTest, RetrieveNonExistentComponent) {
    ComponentRegistry& registry = ComponentRegistry::getInstance();
    Component* retrievedComponent = registry.getComponent("NonExistentComponent");

    EXPECT_EQ(retrievedComponent, nullptr);
}

TEST(ComponentRegistryTest, InitAllComponents) {
    ComponentRegistry& registry = ComponentRegistry::getInstance();
    auto mockComponent = std::make_unique<MockComponent>();
    MockComponent* mockComponentPtr = mockComponent.get();

    registry.registerComponent("TestComponent", std::move(mockComponent));
    registry.initAll();

    ASSERT_EQ(mockComponentPtr->initCount, 1);
}

TEST(ComponentRegistryTest, ShutdownAllComponents) {
    ComponentRegistry& registry = ComponentRegistry::getInstance();
    auto mockComponent = std::make_unique<MockComponent>();
    MockComponent* mockComponentPtr = mockComponent.get();

    registry.registerComponent("TestComponent", std::move(mockComponent));
    registry.shutdownAll();

    ASSERT_EQ(mockComponentPtr->shutdownCount, 1);
}