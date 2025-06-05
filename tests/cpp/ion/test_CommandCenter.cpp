#include "gtest/gtest.h"
#include <gmock/gmock.h>
#include "CommandCenter.h"
#include "EventLoop.h"
#include "Event.h"
#include "GameState.h"
#include "components/CompUnit.h"
#include "commands/Command.h"
#include "ServiceRegistry.h"
#include "GameSettings.h"


using namespace ion;

class MockCommand : public Command {
public:
    MOCK_METHOD(bool, onExecute, (uint32_t, int), (override));
    MOCK_METHOD(bool, onCreateSubCommands, (std::list<Command*>&), (override));
    MOCK_METHOD(std::string, toString, (), (const, override));
    MOCK_METHOD(void, destroy, (), (override));

    void onStart() {};
    void onQueue(uint32_t entityID) {};
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
    }

    void TearDown() override 
    {
        GameState::getInstance().clearAll();
    }
};

TEST_F(CommandCenterTest, HandlesTickEventWithoutCommands) 
{
    Event tickEvent{Event::Type::TICK, TickData{0}};
    ccEventHandlerPtr->onEvent(tickEvent);
    // No assertions needed; just ensure no crashes or exceptions.
}

TEST_F(CommandCenterTest, ExecutesCommandsInQueue_CommandComplete) {
    CompUnit unit;
    auto mockCommand = new MockCommand();
    auto entity = GameState::getInstance().createEntity();

    EXPECT_CALL(*mockCommand, onExecute(entity, 0))
        .WillOnce(::testing::Return(true)); // mark command as completed

    unit.commandQueue.push(mockCommand);
    GameState::getInstance().addComponent(entity, unit);

    Event tickEvent{Event::Type::TICK, TickData{0}};
    ccEventHandlerPtr->onEvent(tickEvent);

    auto unitActual = GameState::getInstance().getComponent<CompUnit>(entity);

    ASSERT_TRUE(unitActual.commandQueue.empty());
}

TEST_F(CommandCenterTest, ExecutesCommandsInQueue_CommandNotComplete) {
    CompUnit unit;
    auto mockCommand = new MockCommand();
    auto entity = GameState::getInstance().createEntity();

    EXPECT_CALL(*mockCommand, onExecute(entity, 0))
        .WillOnce(::testing::Return(false)); // mark command as not completed

    unit.commandQueue.push(mockCommand);
    GameState::getInstance().addComponent(entity, unit);

    Event tickEvent{Event::Type::TICK, TickData{0}};
    ccEventHandlerPtr->onEvent(tickEvent);

    auto unitActual = GameState::getInstance().getComponent<CompUnit>(entity);

    ASSERT_EQ(1, unitActual.commandQueue.size());
}

TEST_F(CommandCenterTest, CreatesSubCommands) {
    CompUnit unit;
    auto mockCommand = new MockCommand();
    auto subCommand = new MockCommand();
    auto entity = GameState::getInstance().createEntity();

    EXPECT_CALL(*mockCommand, onExecute(entity, 0))
        .WillOnce(::testing::Return(false));
    EXPECT_CALL(*mockCommand, onCreateSubCommands(::testing::_))
        .WillOnce(::testing::Invoke([&](std::list<Command*>& newCommands) {
            newCommands.push_back(subCommand);
            return true;
        }));

    unit.commandQueue.push(mockCommand);
    
    GameState::getInstance().addComponent(entity, unit);

    Event tickEvent{Event::Type::TICK, TickData{0}};
    ccEventHandlerPtr->onEvent(tickEvent);

    auto unitActual = GameState::getInstance().getComponent<CompUnit>(entity);

    ASSERT_EQ(unitActual.commandQueue.size(), 2);
}