#include <gtest/gtest.h>
#include <algorithm>
#include <unordered_set>

#include "FogOfWar.h"
#include "Tile.h"
#include "Feet.h"
#include "utils/Constants.h"
#include "utils/Size.h"


namespace core
{
class FogOfWarTest : public ::testing::Test
{
  protected:
    FogOfWar fog;

    void SetUp() override
    {
        // 10x10 test map, start as UNEXPLORED
        fog.init(10, 10, RevealStatus::UNEXPLORED);
    }

    static bool approxEqualReveal(RevealStatus a, RevealStatus b)
    {
        return a == b;
    }

    void expectAll(RevealStatus expected)
    {
        for (int y = 0; y < 10; ++y)
            for (int x = 0; x < 10; ++x)
                EXPECT_TRUE(approxEqualReveal(fog.getRevealStatus(Tile(x, y)), expected))
                    << "Tile (" << x << "," << y << ") differs";
    }
};

TEST_F(FogOfWarTest, InitFillsMap)
{
    // arranged in SetUp
    expectAll(RevealStatus::UNEXPLORED);
}

TEST_F(FogOfWarTest, MarkSingleTileExplored)
{
    Tile t(2, 3);
    fog.markAsExplored(t);

    EXPECT_EQ(fog.getRevealStatus(t), RevealStatus::EXPLORED);
    EXPECT_TRUE(fog.isExplored(t));

    // other tile unaffected
    EXPECT_EQ(fog.getRevealStatus(Tile(0, 0)), RevealStatus::UNEXPLORED);
}

TEST_F(FogOfWarTest, MarkAsExploredFromFeet)
{
    Tile center(4, 4);
    Feet feet = center.toFeet();
    fog.markAsExplored(feet);

    EXPECT_EQ(fog.getRevealStatus(center), RevealStatus::EXPLORED);
}

TEST_F(FogOfWarTest, MarkRadiusFromFeet_Explored)
{
    Tile center(5, 5);
    const uint32_t losTiles = 2;
    const uint32_t losFeet = losTiles * Constants::FEET_PER_TILE;

    fog.markAsExplored(center.toFeet(), losFeet);

    const uint32_t radiusSq = losTiles * losTiles;
    for (int y = 0; y < 10; ++y)
    {
        for (int x = 0; x < 10; ++x)
        {
            int dx = x - center.x;
            int dy = y - center.y;
            if (static_cast<uint32_t>(dx * dx + dy * dy) <= radiusSq)
            {
                EXPECT_EQ(fog.getRevealStatus(Tile(x, y)), RevealStatus::EXPLORED)
                    << "Expected EXPLORED at (" << x << "," << y << ")";
            }
            else
            {
                EXPECT_EQ(fog.getRevealStatus(Tile(x, y)), RevealStatus::UNEXPLORED)
                    << "Expected UNEXPLORED at (" << x << "," << y << ")";
            }
        }
    }
}

TEST_F(FogOfWarTest, MarkRadiusFromTile_Visible)
{
    Tile center(1, 1);
    const uint32_t losTiles = 1;
    const uint32_t losFeet = losTiles * Constants::FEET_PER_TILE;

    fog.markAsVisible(center, losFeet);

    const uint32_t radiusSq = losTiles * losTiles;
    for (int y = 0; y < 10; ++y)
    {
        for (int x = 0; x < 10; ++x)
        {
            int dx = x - center.x;
            int dy = y - center.y;
            if (static_cast<uint32_t>(dx * dx + dy * dy) <= radiusSq)
                EXPECT_EQ(fog.getRevealStatus(Tile(x, y)), RevealStatus::VISIBLE);
            else
                EXPECT_EQ(fog.getRevealStatus(Tile(x, y)), RevealStatus::UNEXPLORED);
        }
    }
}

TEST_F(FogOfWarTest, MarkRadiusWithLandArea_BuildingRectangle)
{
    // Building occupies tiles (3,3),(4,3),(3,4),(4,4)
    LandArea area{{Tile(3, 3), Tile(4, 3), Tile(3, 4), Tile(4, 4)}};
    const uint32_t losTiles = 1;
    const uint32_t losFeet = losTiles * Constants::FEET_PER_TILE;
    std::unordered_set<Tile> tilesShouldExplored;

    for (int x = 2; x <= 5; x++)
        for (int y = 2; y <= 5; y++)
            tilesShouldExplored.insert(Tile(x, y));
    // Centers of corner tiles are little bit more than 1 (LOS), hence cannot be explored
    tilesShouldExplored.erase(Tile(2, 2));
    tilesShouldExplored.erase(Tile(2, 5));
    tilesShouldExplored.erase(Tile(5, 2));
    tilesShouldExplored.erase(Tile(5, 5));

    fog.markAsExplored(area, losFeet);

    for (int y = 0; y < 10; ++y)
    {
        for (int x = 0; x < 10; ++x)
        {
            if (tilesShouldExplored.contains(Tile(x, y)))
            {
                EXPECT_EQ(fog.getRevealStatus(Tile(x, y)), RevealStatus::EXPLORED)
                    << "Expected EXPLORED at (" << x << "," << y << ")";
            }
            else
            {
                EXPECT_EQ(fog.getRevealStatus(Tile(x, y)), RevealStatus::UNEXPLORED)
                    << "Expected UNEXPLORED at (" << x << "," << y << ")";
            }
        }
    }
}
TEST_F(FogOfWarTest, MarkRadiusWithLandArea_ArbitraryShape)
{
    // Create an L-shaped land area
    LandArea area;
    area.tiles.push_back(Tile(2, 2));
    area.tiles.push_back(Tile(2, 3));
    area.tiles.push_back(Tile(3, 3));

    const uint32_t losTiles = 1;
    const uint32_t losFeet = losTiles * Constants::FEET_PER_TILE;

    // note: markAsExplored(feet, area, los) ignores feet and uses area
    fog.markAsExplored(area, losFeet);

    const uint32_t radiusSq = losTiles * losTiles;

    for (int y = 0; y < 10; ++y)
    {
        for (int x = 0; x < 10; ++x)
        {
            // compute min distance to any tile in the area
            uint32_t minDistSq = std::numeric_limits<uint32_t>::max();
            for (const auto& at : area.tiles)
            {
                int dx = x - at.x;
                int dy = y - at.y;
                uint32_t d = static_cast<uint32_t>(dx * dx + dy * dy);
                if (d < minDistSq)
                    minDistSq = d;
            }

            if (minDistSq <= radiusSq)
                EXPECT_EQ(fog.getRevealStatus(Tile(x, y)), RevealStatus::EXPLORED)
                    << "Expected EXPLORED at (" << x << "," << y << ")";
            else
                EXPECT_EQ(fog.getRevealStatus(Tile(x, y)), RevealStatus::UNEXPLORED)
                    << "Expected UNEXPLORED at (" << x << "," << y << ")";
        }
    }
}

TEST_F(FogOfWarTest, LandAreaAtEdges_NoOutOfBounds)
{
    LandArea area;
    area.tiles.push_back(Tile(0, 0));
    area.tiles.push_back(Tile(9, 9));

    const uint32_t losTiles = 2;
    const uint32_t losFeet = losTiles * Constants::FEET_PER_TILE;

    // should not crash and should mark only in-bounds tiles
    fog.markAsExplored(area, losFeet);

    // verify some in-bounds expected tiles
    EXPECT_EQ(fog.getRevealStatus(Tile(0, 0)), RevealStatus::EXPLORED);
    EXPECT_EQ(fog.getRevealStatus(Tile(1, 0)), RevealStatus::EXPLORED); // within radius of (0,0)
    EXPECT_EQ(fog.getRevealStatus(Tile(9, 9)), RevealStatus::EXPLORED);
    EXPECT_EQ(fog.getRevealStatus(Tile(8, 9)), RevealStatus::EXPLORED); // within radius of (9,9)
}

TEST_F(FogOfWarTest, VisibleOverridesExplored)
{
    Tile t(5, 5);
    fog.markAsExplored(t);
    EXPECT_EQ(fog.getRevealStatus(t), RevealStatus::EXPLORED);

    // mark visible (should overwrite)
    fog.markAsVisible(t, Constants::FEET_PER_TILE);
    EXPECT_EQ(fog.getRevealStatus(t), RevealStatus::VISIBLE);
}
} // namespace core