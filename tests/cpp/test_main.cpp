#include <gtest/gtest.h>
#include "game/GameEngine.h"
#include "utils/Logger.h"

TEST(GameEngineTest, StartGame) {
    GameEngine engine;
    EXPECT_NO_THROW(engine.start());
}

TEST(GameEngineTest, UpdateGame) {
    GameEngine engine;
    engine.start();
    EXPECT_NO_THROW(engine.update());
}

TEST(GameEngineTest, RenderGame) {
    GameEngine engine;
    engine.start();
    EXPECT_NO_THROW(engine.render());
}

TEST(LoggerTest, LogInfo) {
    Logger logger;
    EXPECT_NO_THROW(logger.logInfo("This is an info message."));
}

TEST(LoggerTest, LogWarning) {
    Logger logger;
    EXPECT_NO_THROW(logger.logWarning("This is a warning message."));
}

TEST(LoggerTest, LogError) {
    Logger logger;
    EXPECT_NO_THROW(logger.logError("This is an error message."));
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}