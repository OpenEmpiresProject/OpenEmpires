#include "PathFinderAStar.h"
#include "Tile.h"
#include "utils/Constants.h"
#include <gtest/gtest.h>
#include <memory>
#include "Settings.h"
#include "ServiceRegistry.h"
#include "StateManager.h"

namespace core
{

class PathFinderAStarTest : public ::testing::Test
{
  protected:
    PathFinderAStar pathFinder;
    PassabilityMap map;
    Ref<Player> player;

    void SetUp() override
    {
        map.init(5, 5);

        auto settings = std::make_shared<core::Settings>();
        settings->setWorldSizeType(WorldSizeType::TEST);
        ServiceRegistry::getInstance().registerService(settings);

        auto stateMan = std::make_shared<StateManager>();
        ServiceRegistry::getInstance().registerService(stateMan);

        // player used by pathfinder
        player = std::make_shared<Player>();
        player->init(0);

        // Initialize a simple 5x5 passability map for testing:
        // 0 - passable, 1 - blocked
        uint32_t predefined[5][5] = {
            {0, 0, 0, 0, 0}, {0, 1, 1, 1, 0}, {0, 0, 0, 1, 0}, {0, 1, 0, 0, 0}, {0, 0, 0, 0, 0},
        };

        for (int y = 0; y < 5; ++y)
        {
            for (int x = 0; x < 5; ++x)
            {
                if (predefined[y][x] != 0)
                    map.setTileTerrainPassability(Tile(x, y), TerrainPassability::BLOCKED_FOR_ANY);
                else
                    map.setTileTerrainPassability(Tile(x, y), TerrainPassability::PASSABLE_FOR_ANY);
            }
        }
    }
};

TEST_F(PathFinderAStarTest, FindPath_StraightLine)
{
    Feet start = Tile(0, 0).toFeet();
    Feet goal = Tile(4, 0).toFeet();
    Path path = pathFinder.findPath(map, player, start, goal);

    ASSERT_FALSE(path.empty());
    EXPECT_EQ(path.front().toTile(), start.toTile());
    EXPECT_EQ(path.back().toTile(), goal.toTile());
}

TEST_F(PathFinderAStarTest, FindPath_AroundObstacle)
{
    Feet start = Tile(0, 0).toFeet();
    Feet goal = Tile(4, 0).toFeet();
    Path path = pathFinder.findPath(map, player, start, goal);

    ASSERT_FALSE(path.empty());
    EXPECT_EQ(path.front().toTile(), Tile(0, 0));
    EXPECT_EQ(path.back().toTile(), Tile(4, 0));

    // Ensure the path only visits passable tiles for this player
    for (const Feet& pos : path)
    {
        auto tile = pos.toTile();
        EXPECT_TRUE(map.isPassableFor(tile, player->getId()))
            << "Path crosses a blocked tile at " << tile.x << ", " << tile.y;
    }
}

TEST_F(PathFinderAStarTest, FindPath_CornersBlocked)
{
    // Reconfigure map with corner-block pattern
    map.init(5, 5);
    uint32_t predefined[5][5] = {
        {0, 0, 0, 0, 0}, {0, 0, 1, 0, 0}, {0, 1, 0, 1, 0}, {0, 0, 1, 0, 0}, {0, 0, 0, 0, 0},
    };
    for (int y = 0; y < 5; ++y)
        for (int x = 0; x < 5; ++x)
            map.setTileTerrainPassability(Tile(x, y),
                                         predefined[y][x] ? TerrainPassability::BLOCKED_FOR_ANY
                                                          : TerrainPassability::PASSABLE_FOR_ANY);

    Feet start = Tile(2, 2).toFeet(); // inside the block
    Feet goal = Tile(4, 4).toFeet();  // outside the block
    Path path = pathFinder.findPath(map, player, start, goal);

    EXPECT_EQ(path.size(), 1); // no path found, just the starting pos
}

TEST_F(PathFinderAStarTest, FindPath_OnlyOneCornerBlocked)
{
    map.init(5, 5);
    uint32_t predefined[5][5] = {
        {0, 1, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0},
    };
    for (int y = 0; y < 5; ++y)
        for (int x = 0; x < 5; ++x)
            map.setTileTerrainPassability(Tile(x, y),
                                         predefined[y][x] ? TerrainPassability::BLOCKED_FOR_ANY
                                                          : TerrainPassability::PASSABLE_FOR_ANY);

    Feet start = Tile(0, 0).toFeet();
    Feet goal = Tile(4, 4).toFeet();
    Path path = pathFinder.findPath(map, player, start, goal);

    ASSERT_TRUE(path.size() > 1);
}

TEST_F(PathFinderAStarTest, FindPath_StraightLinesUnblocked)
{
    map.init(5, 5);
    uint32_t predefined[5][5] = {
        {0, 0, 0, 0, 0}, {0, 1, 0, 1, 0}, {0, 0, 0, 0, 0}, {0, 1, 0, 1, 0}, {0, 0, 0, 0, 0},
    };
    for (int y = 0; y < 5; ++y)
        for (int x = 0; x < 5; ++x)
            map.setTileTerrainPassability(Tile(x, y),
                                         predefined[y][x] ? TerrainPassability::BLOCKED_FOR_ANY
                                                          : TerrainPassability::PASSABLE_FOR_ANY);

    Feet start = Tile(2, 2).toFeet();
    Feet goal = Tile(4, 4).toFeet();
    Path path = pathFinder.findPath(map, player, start, goal);

    ASSERT_FALSE(path.empty());
}

TEST_F(PathFinderAStarTest, FindPath_NoPathAvailable)
{
    // Block the goal's neighbors to make it unreachable
    map.setTileTerrainPassability(Tile(3, 3), TerrainPassability::BLOCKED_FOR_ANY);
    map.setTileTerrainPassability(Tile(3, 4), TerrainPassability::BLOCKED_FOR_ANY);
    map.setTileTerrainPassability(Tile(4, 3), TerrainPassability::BLOCKED_FOR_ANY);

    Feet start = Tile(0, 0).toFeet();
    Feet goal = Tile(4, 4).toFeet();
    Path path = pathFinder.findPath(map, player, start, goal);

    EXPECT_TRUE(path.size() > 1);                          // Closer to target
    EXPECT_NE(path[path.size() - 1].toTile(), Tile(4, 4)); // but couldn't reach target
}

TEST_F(PathFinderAStarTest, FindPath_StartEqualsGoal)
{
    Feet start = Tile(2, 2).toFeet();
    Feet goal = Tile(2, 2).toFeet();
    Path path = pathFinder.findPath(map, player, start, goal);

    ASSERT_EQ(path.size(), 1);
    EXPECT_EQ(path.front().toTile(), start.toTile());
    EXPECT_EQ(path.back().toTile(), goal.toTile());
}

TEST_F(PathFinderAStarTest, FindPath_LargeMap_StraightLine)
{
    // Initialize a 50x50 passability map with no obstacles
    map.init(50, 50);
    for (int y = 0; y < 50; ++y)
        for (int x = 0; x < 50; ++x)
            map.setTileTerrainPassability(Tile(x, y), TerrainPassability::PASSABLE_FOR_ANY);

    Feet start = Tile(0, 0).toFeet();
    Feet goal = Tile(49, 0).toFeet();
    Path path = pathFinder.findPath(map, player, start, goal);

    ASSERT_FALSE(path.empty());
    EXPECT_EQ(path.front().toTile(), start.toTile());
    EXPECT_EQ(path.back().toTile(), goal.toTile());
}

TEST_F(PathFinderAStarTest, FindPath_LargeMap_WithObstacles)
{
    // Initialize a 50x50 map with a diagonal wall of obstacles
    map.init(50, 50);
    for (int y = 0; y < 50; ++y)
        for (int x = 0; x < 50; ++x)
            map.setTileTerrainPassability(Tile(x, y), TerrainPassability::PASSABLE_FOR_ANY);

    for (int i = 0; i < 50; ++i)
        map.setTileTerrainPassability(Tile(i, i), TerrainPassability::BLOCKED_FOR_ANY);

    // Make start and goal tiles passable
    map.setTileTerrainPassability(Tile(0, 0), TerrainPassability::PASSABLE_FOR_ANY);
    map.setTileTerrainPassability(Tile(49, 49), TerrainPassability::PASSABLE_FOR_ANY);

    Feet start = Tile(0, 49).toFeet();
    Feet goal = Tile(49, 0).toFeet();
    Path path = pathFinder.findPath(map, player, start, goal);

    ASSERT_FALSE(path.empty());
    EXPECT_EQ(path.front().toTile(), start.toTile());
    EXPECT_EQ(path.back().toTile(), goal.toTile());

    // Ensure the path avoids blocked tiles
    for (const Feet& pos : path)
    {
        auto tile = pos.toTile();
        EXPECT_TRUE(map.isPassableFor(tile, player->getId()))
            << "Path crosses a blocked tile at " << tile.x << ", " << tile.y;
    }
}

TEST_F(PathFinderAStarTest, FindPath_LargeMap_TargetBlocked)
{
    // Initialize a 50x50 map and block the goal region completely
    map.init(50, 50);
    for (int y = 0; y < 50; ++y)
        for (int x = 0; x < 50; ++x)
            map.setTileTerrainPassability(Tile(x, y), TerrainPassability::PASSABLE_FOR_ANY);

    // Block goal 49,49 and its neighbors
    map.setTileTerrainPassability(Tile(48, 48), TerrainPassability::BLOCKED_FOR_ANY);
    map.setTileTerrainPassability(Tile(48, 49), TerrainPassability::BLOCKED_FOR_ANY);
    map.setTileTerrainPassability(Tile(49, 48), TerrainPassability::BLOCKED_FOR_ANY);
    map.setTileTerrainPassability(Tile(49, 49), TerrainPassability::BLOCKED_FOR_ANY);

    Feet start = Tile(0, 0).toFeet();
    Feet goal = Tile(49, 49).toFeet();
    Path path = pathFinder.findPath(map, player, start, goal);

    EXPECT_TRUE(path.size() > 40);                           // Path got closer to the target
    EXPECT_NE(path[path.size() - 1].toTile(), Tile(49, 49)); // but not exactly the target
}

TEST_F(PathFinderAStarTest, FindPath_LargeMap_StartEqualsGoal)
{
    // Initialize a 50x50 map with no obstacles
    map.init(50, 50);
    for (int y = 0; y < 50; ++y)
        for (int x = 0; x < 50; ++x)
            map.setTileTerrainPassability(Tile(x, y), TerrainPassability::PASSABLE_FOR_ANY);

    Feet start = Tile(25, 25).toFeet();
    Feet goal = Tile(25, 25).toFeet();
    Path path = pathFinder.findPath(map, player, start, goal);

    ASSERT_EQ(path.size(), 1);
    EXPECT_EQ(path.front().toTile(), start.toTile());
    EXPECT_EQ(path.back().toTile(), goal.toTile());
}
} // namespace core