#include "PassabilityMap.h"
#include "Tile.h"

#include <gtest/gtest.h>

namespace core
{

class PassabilityMapTest : public ::testing::Test
{
  protected:
    PassabilityMap map;

    void SetUp() override
    {
        // Small map for tests
        map.init(3, 3);
    }
};

TEST_F(PassabilityMapTest, DefaultTilesArePassableForAnyPlayer)
{
    Tile t(1, 1);
    // Defaults: terrain PASSABLE_FOR_ANY, dynamic PASSABLE_FOR_ANY
    EXPECT_TRUE(map.isPassableFor(t, 0));
    EXPECT_TRUE(map.isPassableFor(t, 42));
}

TEST_F(PassabilityMapTest, TerrainBlockedMakesTileUnpassable)
{
    Tile t(0, 0);
    map.setTileTerrainPassability(t, TerrainPassability::BLOCKED_FOR_ANY);
    EXPECT_FALSE(map.isPassableFor(t, 1));

    // Other terrain types (currently only PASSABLE_FOR_ANY is treated as passable)
    map.setTileTerrainPassability(t, TerrainPassability::PASSABLE_FOR_LAND_UNITS);
    EXPECT_FALSE(map.isPassableFor(t, 1));

    map.setTileTerrainPassability(t, TerrainPassability::PASSABLE_FOR_WATER_UNITS);
    EXPECT_FALSE(map.isPassableFor(t, 1));
}

TEST_F(PassabilityMapTest, DynamicBlockedMakesTileUnpassable)
{
    Tile t(1, 0);
    // Terrain remains PASSABLE_FOR_ANY by default
    map.setTileDynamicPassability(t, DynamicPassability::BLOCKED_FOR_ANY);
    EXPECT_FALSE(map.isPassableFor(t, 0));
    EXPECT_FALSE(map.isPassableFor(t, 7));
}

TEST_F(PassabilityMapTest, OwnerOnlyPassableForOwnerOnly)
{
    Tile t(2, 2);
    // Set dynamic passability to owner-only for player 5
    map.setTileDynamicPassability(t, DynamicPassability::PASSABLE_FOR_OWNER_OR_ALLIED, /*owner=*/5);

    // Owner can pass
    EXPECT_TRUE(map.isPassableFor(t, 5));

    // Other players cannot
    EXPECT_FALSE(map.isPassableFor(t, 0));
    EXPECT_FALSE(map.isPassableFor(t, 6));
}

TEST_F(PassabilityMapTest, OwnerOnlyButTerrainBlockedRemainsUnpassable)
{
    Tile t(0, 2);
    map.setTileDynamicPassability(t, DynamicPassability::PASSABLE_FOR_OWNER_OR_ALLIED, /*owner=*/2);
    map.setTileTerrainPassability(t, TerrainPassability::BLOCKED_FOR_ANY);

    // Terrain blocks movement regardless of dynamic ownership
    EXPECT_FALSE(map.isPassableFor(t, 2));
}

TEST_F(PassabilityMapTest, ChangeDynamicPassabilityToPassableRestoresAccess)
{
    Tile t(1, 2);
    map.setTileDynamicPassability(t, DynamicPassability::BLOCKED_FOR_ANY);
    EXPECT_FALSE(map.isPassableFor(t, 3));

    // Now make it passable again
    map.setTileDynamicPassability(t, DynamicPassability::PASSABLE_FOR_ANY);
    EXPECT_TRUE(map.isPassableFor(t, 3));
}
} // namespace core