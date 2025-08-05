#include "Coordinates.h"
#include "GameSettings.h"
#include "Feet.h"
#include "utils/Size.h"
#include "utils/Constants.h"
#include <gtest/gtest.h>
#include <SDL3/SDL_events.h>
#include <mutex>

namespace core
{

// Mock GameSettings for testing
class MockGameSettings : public GameSettings
{
  public:
    MockGameSettings()
    {
        setWorldSizeType(WorldSizeType::DEMO);
        setViewportMovingSpeed(10);
    }
};

// Test fixture for Coordinates
class ViewportTest : public ::testing::Test
{
  protected:
    std::shared_ptr<MockGameSettings> settings = std::make_shared<MockGameSettings>();
    Coordinates coordinates;

    ViewportTest() : coordinates(settings)
    {
    }

    void SetUp() override
    {
        coordinates.setViewportPositionInPixels(Vec2(0, 0));
    }
};

TEST_F(ViewportTest, InitialViewportPosition)
{
    EXPECT_EQ(coordinates.getViewportPositionInPixels(), Vec2(0, 0));
}

// TEST_F(ViewportTest, RequestPositionChange) {
//     coordinates.requestPositionChange(Feet(10, 20));
//     EXPECT_TRUE(coordinates.isPositionChangeRequested());
// }

// TEST_F(ViewportTest, SyncPosition) {
//     coordinates.requestPositionChange(Feet(10, 20));
//     coordinates.syncPosition();
//     EXPECT_EQ(coordinates.getViewportPositionInPixels(), Feet(10, 20));
//     EXPECT_FALSE(coordinates.isPositionChangeRequested());
// }

TEST_F(ViewportTest, FeetToPixelsConversion_TopCorner)
{
    Feet feet(0, 0);
    auto pixels = coordinates.feetToPixels(feet);
    EXPECT_EQ(pixels, Vec2(Constants::TILE_PIXEL_WIDTH * 50 / 2, 0));
}

TEST_F(ViewportTest, FeetToPixelsConversion_LeftCorner)
{
    Feet feet(0, 50 * Constants::FEET_PER_TILE);
    auto pixels = coordinates.feetToPixels(feet);
    EXPECT_EQ(pixels, Vec2(0, Constants::TILE_PIXEL_HEIGHT * 50 / 2));
}

TEST_F(ViewportTest, FeetToPixelsConversion_RightCorner)
{
    Feet feet(50 * Constants::FEET_PER_TILE, 0);
    auto pixels = coordinates.feetToPixels(feet);
    EXPECT_EQ(pixels,
              Vec2(Constants::TILE_PIXEL_WIDTH * 50, Constants::TILE_PIXEL_HEIGHT * 50 / 2));
}

TEST_F(ViewportTest, FeetToPixelsConversion_BottomCorner)
{
    Feet feet(50 * Constants::FEET_PER_TILE, 50 * Constants::FEET_PER_TILE);
    auto pixels = coordinates.feetToPixels(feet);
    EXPECT_EQ(pixels,
              Vec2(Constants::TILE_PIXEL_WIDTH * 50 / 2, Constants::TILE_PIXEL_HEIGHT * 50));
}

// Testing integer division
TEST_F(ViewportTest, FeetToPixelsConversion_SubTile)
{
    Feet feet(128, 128);
    auto pixels = coordinates.feetToPixels(feet);
    EXPECT_EQ(pixels, Vec2(Constants::TILE_PIXEL_WIDTH * 50 / 2, 24));
}

// Testing integer division
TEST_F(ViewportTest, FeetToPixelsConversion_SubTile2)
{
    Feet feet(64, 64);
    auto pixels = coordinates.feetToPixels(feet);
    EXPECT_EQ(pixels, Vec2(Constants::TILE_PIXEL_WIDTH * 50 / 2, 12));
}

TEST_F(ViewportTest, PixelsToFeetConversion_TopCorner)
{
    Vec2 pixels(Constants::TILE_PIXEL_WIDTH * 50 / 2, 0);
    Feet feet = coordinates.pixelsToFeet(pixels);
    EXPECT_EQ(feet, Feet(0, 0));
}

TEST_F(ViewportTest, PixelsToFeetConversion_LeftCorner)
{
    Vec2 pixels(0, Constants::TILE_PIXEL_HEIGHT * 50 / 2);
    Feet feet = coordinates.pixelsToFeet(pixels);
    EXPECT_EQ(feet, Feet(0, 50 * Constants::FEET_PER_TILE));
}

TEST_F(ViewportTest, PixelsToFeetConversion_RightCorner)
{
    Vec2 pixels(Constants::TILE_PIXEL_WIDTH * 50, Constants::TILE_PIXEL_HEIGHT * 50 / 2);
    Feet feet = coordinates.pixelsToFeet(pixels);
    EXPECT_EQ(feet, Feet(50 * Constants::FEET_PER_TILE, 0));
}

TEST_F(ViewportTest, PixelsToFeetConversion_BottomCorner)
{
    Vec2 pixels(Constants::TILE_PIXEL_WIDTH * 50 / 2, Constants::TILE_PIXEL_HEIGHT * 50);
    Feet feet = coordinates.pixelsToFeet(pixels);
    EXPECT_EQ(feet, Feet(50 * Constants::FEET_PER_TILE, 50 * Constants::FEET_PER_TILE));
}

// Testing integer division
TEST_F(ViewportTest, PixelsToFeetConversion_SubTile)
{
    Vec2 pixels(Constants::TILE_PIXEL_WIDTH * 50 / 2, 24);
    Feet feet = coordinates.pixelsToFeet(pixels);
    EXPECT_EQ(feet, Feet(128, 128));
}

// Testing integer division
TEST_F(ViewportTest, PixelsToFeetConversion_SubTile2)
{
    Vec2 pixels(Constants::TILE_PIXEL_WIDTH * 50 / 2, 12);
    Feet feet = coordinates.pixelsToFeet(pixels);
    EXPECT_EQ(feet, Feet(64, 64));
}

TEST_F(ViewportTest, PixelsToScreenUnitsConversion)
{
    coordinates.setViewportPositionInPixels(Vec2(50, 50));
    Vec2 pixels(100, 100);
    EXPECT_EQ(coordinates.pixelsToScreenUnits(pixels), Vec2(50, 50));
}

TEST_F(ViewportTest, ScreenUnitsToPixelsConversion)
{
    coordinates.setViewportPositionInPixels(Vec2(50, 50));
    Vec2 screenUnits(50, 50);
    EXPECT_EQ(coordinates.screenUnitsToPixels(screenUnits), Vec2(100, 100));
}

// TEST_F(ViewportTest, HandleKeyDownEvent) {
//     SDL_Event sdlEvent;
//     sdlEvent.key.key = SDLK_A; // Simulate 'A' key press
//     Event event(Event::Type::KEY_DOWN, &sdlEvent);

//     EventHandler* eventHandler = &coordinates;

//     eventHandler->onEvent(event);
//     coordinates.syncPosition();
//     EXPECT_EQ(coordinates.getViewportPositionInPixels(), Feet(-settings.getViewportMovingSpeed(),
//     0));
//
} // namespace core