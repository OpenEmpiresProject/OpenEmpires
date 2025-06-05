#include "Coordinates.h"
#include "GameSettings.h"
#include "Vec2d.h"
#include "utils/Size.h"
#include "utils/Constants.h"
#include <gtest/gtest.h>
#include <SDL3/SDL_events.h>
#include <mutex>

using namespace ion;

// Mock GameSettings for testing
class MockGameSettings : public GameSettings {
public:
    MockGameSettings() {
        setWorldSizeType(WorldSizeType::DEMO);
        setViewportMovingSpeed(10);
    }
};

// Test fixture for Coordinates
class ViewportTest : public ::testing::Test {
protected:
    std::shared_ptr<MockGameSettings> settings = std::make_shared<MockGameSettings>();
    Coordinates coordinates;

    ViewportTest() : coordinates(settings) {}

    void SetUp() override {
        coordinates.setViewportPositionInPixels(Vec2d(0, 0));
    }
};

TEST_F(ViewportTest, InitialViewportPosition) {
    EXPECT_EQ(coordinates.getViewportPositionInPixels(), Vec2d(0, 0));
}

// TEST_F(ViewportTest, RequestPositionChange) {
//     coordinates.requestPositionChange(Vec2d(10, 20));
//     EXPECT_TRUE(coordinates.isPositionChangeRequested());
// }

// TEST_F(ViewportTest, SyncPosition) {
//     coordinates.requestPositionChange(Vec2d(10, 20));
//     coordinates.syncPosition();
//     EXPECT_EQ(coordinates.getViewportPositionInPixels(), Vec2d(10, 20));
//     EXPECT_FALSE(coordinates.isPositionChangeRequested());
// }

TEST_F(ViewportTest, FeetToPixelsConversion_TopCorner) {
    Vec2d feet(0, 0);
    Vec2d pixels = coordinates.feetToPixels(feet);
    EXPECT_EQ(pixels, Vec2d(Constants::TILE_PIXEL_WIDTH * 50 / 2, 0));
}

TEST_F(ViewportTest, FeetToPixelsConversion_LeftCorner) {
    Vec2d feet(0, 50 * Constants::FEET_PER_TILE);
    Vec2d pixels = coordinates.feetToPixels(feet);
    EXPECT_EQ(pixels, Vec2d(0, Constants::TILE_PIXEL_HEIGHT * 50 / 2));
}

TEST_F(ViewportTest, FeetToPixelsConversion_RightCorner) {
    Vec2d feet(50 * Constants::FEET_PER_TILE, 0);
    Vec2d pixels = coordinates.feetToPixels(feet);
    EXPECT_EQ(pixels, Vec2d(Constants::TILE_PIXEL_WIDTH * 50, Constants::TILE_PIXEL_HEIGHT * 50 / 2));
}

TEST_F(ViewportTest, FeetToPixelsConversion_BottomCorner) {
    Vec2d feet(50 * Constants::FEET_PER_TILE, 50 * Constants::FEET_PER_TILE);
    Vec2d pixels = coordinates.feetToPixels(feet);
    EXPECT_EQ(pixels, Vec2d(Constants::TILE_PIXEL_WIDTH * 50 / 2, Constants::TILE_PIXEL_HEIGHT * 50));
}

// Testing integer division
TEST_F(ViewportTest, FeetToPixelsConversion_SubTile)
{
    Vec2d feet(128, 128);
    Vec2d pixels = coordinates.feetToPixels(feet);
    EXPECT_EQ(pixels, Vec2d(Constants::TILE_PIXEL_WIDTH * 50 / 2, 24));
}

// Testing integer division
TEST_F(ViewportTest, FeetToPixelsConversion_SubTile2)
{
    Vec2d feet(64, 64);
    Vec2d pixels = coordinates.feetToPixels(feet);
    EXPECT_EQ(pixels, Vec2d(Constants::TILE_PIXEL_WIDTH * 50 / 2, 12));
}

TEST_F(ViewportTest, PixelsToFeetConversion_TopCorner) {
    Vec2d pixels(Constants::TILE_PIXEL_WIDTH * 50 / 2, 0);
    Vec2d feet = coordinates.pixelsToFeet(pixels);
    EXPECT_EQ(feet, Vec2d(0, 0));
}

TEST_F(ViewportTest, PixelsToFeetConversion_LeftCorner) {
    Vec2d pixels(0, Constants::TILE_PIXEL_HEIGHT * 50 / 2);
    Vec2d feet = coordinates.pixelsToFeet(pixels);
    EXPECT_EQ(feet, Vec2d(0, 50 * Constants::FEET_PER_TILE));
}

TEST_F(ViewportTest, PixelsToFeetConversion_RightCorner) {
    Vec2d pixels(Constants::TILE_PIXEL_WIDTH * 50, Constants::TILE_PIXEL_HEIGHT * 50 / 2);
    Vec2d feet = coordinates.pixelsToFeet(pixels);
    EXPECT_EQ(feet, Vec2d(50 * Constants::FEET_PER_TILE, 0));
}

TEST_F(ViewportTest, PixelsToFeetConversion_BottomCorner) {
    Vec2d pixels(Constants::TILE_PIXEL_WIDTH * 50 / 2, Constants::TILE_PIXEL_HEIGHT * 50);
    Vec2d feet = coordinates.pixelsToFeet(pixels);
    EXPECT_EQ(feet, Vec2d(50 * Constants::FEET_PER_TILE, 50 * Constants::FEET_PER_TILE));
}

// Testing integer division
TEST_F(ViewportTest, PixelsToFeetConversion_SubTile) {
    Vec2d pixels(Constants::TILE_PIXEL_WIDTH * 50 / 2, 24);
    Vec2d feet = coordinates.pixelsToFeet(pixels);
    EXPECT_EQ(feet, Vec2d(128, 128));
}

// Testing integer division
TEST_F(ViewportTest, PixelsToFeetConversion_SubTile2) {
    Vec2d pixels(Constants::TILE_PIXEL_WIDTH * 50 / 2, 12);
    Vec2d feet = coordinates.pixelsToFeet(pixels);
    EXPECT_EQ(feet, Vec2d(64, 64));
}

TEST_F(ViewportTest, PixelsToScreenUnitsConversion) {
    coordinates.setViewportPositionInPixels(Vec2d(50, 50));
    Vec2d pixels(100, 100);
    EXPECT_EQ(coordinates.pixelsToScreenUnits(pixels), Vec2d(50, 50));
}

TEST_F(ViewportTest, ScreenUnitsToPixelsConversion) {
    coordinates.setViewportPositionInPixels(Vec2d(50, 50));
    Vec2d screenUnits(50, 50);
    EXPECT_EQ(coordinates.screenUnitsToPixels(screenUnits), Vec2d(100, 100));
}

// TEST_F(ViewportTest, HandleKeyDownEvent) {
//     SDL_Event sdlEvent;
//     sdlEvent.key.key = SDLK_A; // Simulate 'A' key press
//     Event event(Event::Type::KEY_DOWN, &sdlEvent);

//     EventHandler* eventHandler = &coordinates;

//     eventHandler->onEvent(event);
//     coordinates.syncPosition();
//     EXPECT_EQ(coordinates.getViewportPositionInPixels(), Vec2d(-settings.getViewportMovingSpeed(), 0));
// }