#include "gtest/gtest.h"
#include <gmock/gmock.h>
#include "CommandCenter.h"
#include "EventLoop.h"
#include "Event.h"
#include "GameState.h"
#include "components/CompUnit.h"
#include "commands/Command.h"
#include "components/CompAction.h"
#include "components/CompAnimation.h"
#include "components/CompDirty.h"
#include "components/CompEntityInfo.h"
#include "components/CompGraphics.h"
#include "components/CompPlayer.h"
#include "components/CompTransform.h"

#include "ServiceRegistry.h"
#include "GameSettings.h"

namespace core
{

class MockCommand : public Command
{
  public:
    MOCK_METHOD(bool, onExecute, (int, std::list<Command*>&), (override));
    MOCK_METHOD(std::string, toString, (), (const, override));
    MOCK_METHOD(void, destroy, (), (override));

    void onStart() {};
    void onQueue() {};
};

class CommandCenterTest : public ::testing::Test
{

  protected:
    CommandCenter commandCenter;
    EventHandler* ccEventHandlerPtr;

    void SetUp() override
    {
        ccEventHandlerPtr = &commandCenter;
        ServiceRegistry::getInstance().registerService(std::make_shared<GameSettings>());

        auto gameState = std::make_shared<core::GameState>();
        ServiceRegistry::getInstance().registerService(gameState);
    }

    void TearDown() override
    {
        ServiceRegistry::getInstance().getService<GameState>()->clearAll();
    }
};

TEST_F(CommandCenterTest, HandlesTickEventWithoutCommands)
{
    Event tickEvent{Event::Type::TICK, TickData{0}};
    ccEventHandlerPtr->onEvent(tickEvent);
    // No assertions needed; just ensure no crashes or exceptions.
}

TEST_F(CommandCenterTest, ExecutesCommandsInQueue_CommandComplete)
{
    CompUnit unit;
    auto mockCommand = new MockCommand();
    auto entity = ServiceRegistry::getInstance().getService<GameState>()->createEntity();
    std::list<Command*> subCommands;

    EXPECT_CALL(*mockCommand, onExecute(0, subCommands))
        .WillOnce(::testing::Return(true)); // mark command as completed

    unit.commandQueue.push(mockCommand);
    ServiceRegistry::getInstance().getService<GameState>()->addComponent(entity, unit);

    Event tickEvent{Event::Type::TICK, TickData{0}};
    ccEventHandlerPtr->onEvent(tickEvent);

    auto unitActual =
        ServiceRegistry::getInstance().getService<GameState>()->getComponent<CompUnit>(entity);

    ASSERT_TRUE(unitActual.commandQueue.empty());
}

TEST_F(CommandCenterTest, ExecutesCommandsInQueue_CommandNotComplete)
{
    CompUnit unit;
    auto mockCommand = new MockCommand();
    auto entity = ServiceRegistry::getInstance().getService<GameState>()->createEntity();
    std::list<Command*> subCommands;

    EXPECT_CALL(*mockCommand, onExecute(0, subCommands))
        .WillOnce(::testing::Return(false)); // mark command as not completed

    unit.commandQueue.push(mockCommand);
    ServiceRegistry::getInstance().getService<GameState>()->addComponent(entity, unit);

    Event tickEvent{Event::Type::TICK, TickData{0}};
    ccEventHandlerPtr->onEvent(tickEvent);

    auto unitActual =
        ServiceRegistry::getInstance().getService<GameState>()->getComponent<CompUnit>(entity);

    ASSERT_EQ(1, unitActual.commandQueue.size());
}

TEST_F(CommandCenterTest, CreatesSubCommands)
{
    CompUnit unit;
    auto mockCommand = new MockCommand();
    auto subCommand = new MockCommand();
    auto entity = ServiceRegistry::getInstance().getService<GameState>()->createEntity();
    std::list<Command*> subCommands;

    EXPECT_CALL(*mockCommand, onExecute(0, subCommands))
        .WillOnce(::testing::Invoke(
            [&](int deltaTimeMs, std::list<Command*>& newCommands)
            {
                newCommands.push_back(subCommand);
                return false;
            }));

    unit.commandQueue.push(mockCommand);

    ServiceRegistry::getInstance().getService<GameState>()->addComponent(entity, unit);
    ServiceRegistry::getInstance().getService<GameState>()->addComponent(entity, CompAction(0));
    ServiceRegistry::getInstance().getService<GameState>()->addComponent(entity, CompAnimation());
    ServiceRegistry::getInstance().getService<GameState>()->addComponent(entity, CompDirty());
    ServiceRegistry::getInstance().getService<GameState>()->addComponent(entity, CompEntityInfo(0));
    ServiceRegistry::getInstance().getService<GameState>()->addComponent(entity, CompPlayer());
    ServiceRegistry::getInstance().getService<GameState>()->addComponent(entity, CompTransform());

    Event tickEvent{Event::Type::TICK, TickData{0}};
    ccEventHandlerPtr->onEvent(tickEvent);

    auto unitActual =
        ServiceRegistry::getInstance().getService<GameState>()->getComponent<CompUnit>(entity);

    ASSERT_EQ(unitActual.commandQueue.size(), 2);
}
} // namespace core