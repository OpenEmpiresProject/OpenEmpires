#include "Viewport.h"
#include "GameSettings.h"
#include "Vec2d.h"
#include "WidthHeight.h"
#include "Constants.h"
#include <gtest/gtest.h>
#include <SDL3/SDL_events.h>
#include <mutex>

using namespace aion;
using namespace utils;

// Mock GameSettings for testing
class MockGameSettings : public GameSettings {
public:
    MockGameSettings() {
        setWorldSizeType(utils::WorldSizeType::DEMO);
        setViewportMovingSpeed(10);
    }
};

// Test fixture for Viewport
class ViewportTest : public ::testing::Test {
protected:
    MockGameSettings settings;
    Viewport viewport;

    ViewportTest() : viewport(settings) {}

    void SetUp() override {
        viewport.setViewportPositionInPixels(Vec2d(0, 0));
    }
};

TEST_F(ViewportTest, InitialViewportPosition) {
    EXPECT_EQ(viewport.getViewportPositionInPixels(), Vec2d(0, 0));
}

TEST_F(ViewportTest, RequestPositionChange) {
    viewport.requestPositionChange(Vec2d(10, 20));
    EXPECT_TRUE(viewport.isPositionChangeRequested());
}

TEST_F(ViewportTest, SyncPosition) {
    viewport.requestPositionChange(Vec2d(10, 20));
    viewport.syncPosition();
    EXPECT_EQ(viewport.getViewportPositionInPixels(), Vec2d(10, 20));
    EXPECT_FALSE(viewport.isPositionChangeRequested());
}

TEST_F(ViewportTest, FeetToPixelsConversion_TopCorner) {
    Vec2d feet(0, 0);
    Vec2d pixels = viewport.feetToPixels(feet);
    EXPECT_EQ(pixels, Vec2d(utils::Constants::TILE_PIXEL_WIDTH * 50 / 2, 0));
}

TEST_F(ViewportTest, FeetToPixelsConversion_LeftCorner) {
    Vec2d feet(0, 50 * utils::Constants::FEET_PER_TILE);
    Vec2d pixels = viewport.feetToPixels(feet);
    EXPECT_EQ(pixels, Vec2d(0, utils::Constants::TILE_PIXEL_HEIGHT * 50 / 2));
}

TEST_F(ViewportTest, FeetToPixelsConversion_RightCorner) {
    Vec2d feet(50 * utils::Constants::FEET_PER_TILE, 0);
    Vec2d pixels = viewport.feetToPixels(feet);
    EXPECT_EQ(pixels, Vec2d(utils::Constants::TILE_PIXEL_WIDTH * 50, utils::Constants::TILE_PIXEL_HEIGHT * 50 / 2));
}

TEST_F(ViewportTest, FeetToPixelsConversion_BottomCorner) {
    Vec2d feet(50 * utils::Constants::FEET_PER_TILE, 50 * utils::Constants::FEET_PER_TILE);
    Vec2d pixels = viewport.feetToPixels(feet);
    EXPECT_EQ(pixels, Vec2d(utils::Constants::TILE_PIXEL_WIDTH * 50 / 2, utils::Constants::TILE_PIXEL_HEIGHT * 50));
}

// Testing integer division
TEST_F(ViewportTest, FeetToPixelsConversion_SubTile)
{
    Vec2d feet(128, 128);
    Vec2d pixels = viewport.feetToPixels(feet);
    EXPECT_EQ(pixels, Vec2d(utils::Constants::TILE_PIXEL_WIDTH * 50 / 2, 16));
}

// Testing integer division
TEST_F(ViewportTest, FeetToPixelsConversion_SubTile2)
{
    Vec2d feet(64, 64);
    Vec2d pixels = viewport.feetToPixels(feet);
    EXPECT_EQ(pixels, Vec2d(utils::Constants::TILE_PIXEL_WIDTH * 50 / 2, 8));
}

// Testing integer division
TEST_F(ViewportTest, FeetToPixelsConversion_SubTile3)
{
    Vec2d feet(96, 64);
    Vec2d pixels = viewport.feetToPixels(feet);
    EXPECT_EQ(pixels, Vec2d(utils::Constants::TILE_PIXEL_WIDTH * 50 / 2 + 4, 8 + 2));
}

TEST_F(ViewportTest, PixelsToFeetConversion_TopCorner) {
    Vec2d pixels(utils::Constants::TILE_PIXEL_WIDTH * 50 / 2, 0);
    Vec2d feet = viewport.pixelsToFeet(pixels);
    EXPECT_EQ(feet, Vec2d(0, 0));
}

TEST_F(ViewportTest, PixelsToFeetConversion_LeftCorner) {
    Vec2d pixels(0, utils::Constants::TILE_PIXEL_HEIGHT * 50 / 2);
    Vec2d feet = viewport.pixelsToFeet(pixels);
    EXPECT_EQ(feet, Vec2d(0, 50 * utils::Constants::FEET_PER_TILE));
}

TEST_F(ViewportTest, PixelsToFeetConversion_RightCorner) {
    Vec2d pixels(utils::Constants::TILE_PIXEL_WIDTH * 50, utils::Constants::TILE_PIXEL_HEIGHT * 50 / 2);
    Vec2d feet = viewport.pixelsToFeet(pixels);
    EXPECT_EQ(feet, Vec2d(50 * utils::Constants::FEET_PER_TILE, 0));
}

TEST_F(ViewportTest, PixelsToFeetConversion_BottomCorner) {
    Vec2d pixels(utils::Constants::TILE_PIXEL_WIDTH * 50 / 2, utils::Constants::TILE_PIXEL_HEIGHT * 50);
    Vec2d feet = viewport.pixelsToFeet(pixels);
    EXPECT_EQ(feet, Vec2d(50 * utils::Constants::FEET_PER_TILE, 50 * utils::Constants::FEET_PER_TILE));
}

// Testing integer division
TEST_F(ViewportTest, PixelsToFeetConversion_SubTile) {
    Vec2d pixels(utils::Constants::TILE_PIXEL_WIDTH * 50 / 2, 16);
    Vec2d feet = viewport.pixelsToFeet(pixels);
    EXPECT_EQ(feet, Vec2d(128, 128));
}

// Testing integer division
TEST_F(ViewportTest, PixelsToFeetConversion_SubTile2) {
    Vec2d pixels(utils::Constants::TILE_PIXEL_WIDTH * 50 / 2, 8);
    Vec2d feet = viewport.pixelsToFeet(pixels);
    EXPECT_EQ(feet, Vec2d(64, 64));
}

// Testing integer division
TEST_F(ViewportTest, PixelsToFeetConversion_SubTile3)
{
    Vec2d pixels(utils::Constants::TILE_PIXEL_WIDTH * 50 / 2 + 4, 8 + 2);
    Vec2d feet = viewport.pixelsToFeet(pixels);
    EXPECT_EQ(feet, Vec2d(96, 64));
}

TEST_F(ViewportTest, PixelsToScreenUnitsConversion) {
    viewport.setViewportPositionInPixels(Vec2d(50, 50));
    Vec2d pixels(100, 100);
    EXPECT_EQ(viewport.pixelsToScreenUnits(pixels), Vec2d(50, 50));
}

TEST_F(ViewportTest, ScreenUnitsToPixelsConversion) {
    viewport.setViewportPositionInPixels(Vec2d(50, 50));
    Vec2d screenUnits(50, 50);
    EXPECT_EQ(viewport.screenUnitsToPixels(screenUnits), Vec2d(100, 100));
}

TEST_F(ViewportTest, HandleKeyDownEvent) {
    SDL_Event sdlEvent;
    sdlEvent.key.key = SDLK_A; // Simulate 'A' key press
    Event event(Event::Type::KEY_DOWN, &sdlEvent);

    EventHandler* eventHandler = &viewport;

    eventHandler->onEvent(event);
    viewport.syncPosition();
    EXPECT_EQ(viewport.getViewportPositionInPixels(), Vec2d(-settings.getViewportMovingSpeed(), 0));
}