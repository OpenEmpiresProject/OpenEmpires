#include "PathFinderAStar.h"
#include "Tile.h"
#include "utils/Constants.h"
#include <gtest/gtest.h>

namespace ion
{

class PathFinderAStarTest : public ::testing::Test
{
  protected:
    PathFinderAStar pathFinder;
    TileMap map;

    void SetUp() override
    {
        map.init(5, 5);

        // Initialize a simple 5x5 map for testing
        uint32_t predefined[5][5] = {
            {0, 0, 0, 0, 0}, {0, 1, 1, 1, 0}, {0, 0, 0, 1, 0}, {0, 1, 0, 0, 0}, {0, 0, 0, 0, 0},
        };

        for (int i = 0; i < 5; ++i)
        {
            for (int j = 0; j < 5; ++j)
            {
                if (predefined[i][j] != 0)
                    map.getStaticMap()[i][j].addEntity(predefined[i][j]);
            }
        }
    }
};

TEST_F(PathFinderAStarTest, FindPath_StraightLine)
{
    Feet start = Tile(0, 0).toFeet();
    Feet goal = Tile(4, 0).toFeet();
    Path path = pathFinder.findPath(map, start, goal);

    ASSERT_FALSE(path.empty());
    EXPECT_EQ(path.front().toTile(), start.toTile());
    EXPECT_EQ(path.back().toTile(), goal.toTile());
}

TEST_F(PathFinderAStarTest, FindPath_AroundObstacle)
{
    Feet start = Tile(0, 0).toFeet();
    Feet goal = Tile(4, 0).toFeet();
    Path path = pathFinder.findPath(map, start, goal);

    ASSERT_FALSE(path.empty());
    EXPECT_EQ(path.front().toTile(), start.toTile());
    EXPECT_EQ(path.back().toTile(), goal.toTile());

    // Ensure the path avoids obstacles
    for (const Feet& pos : path)
    {
        auto tile = pos.toTile();
        EXPECT_TRUE(map.getStaticMap()[tile.x][tile.y].isOccupied() == false)
            << "Path crosses an obstacle at " << tile.x << ", " << tile.y;
    }
}

TEST_F(PathFinderAStarTest, FindPath_CornersBlocked)
{
    uint32_t predefined[5][5] = {
        {0, 0, 0, 0, 0}, {0, 0, 1, 0, 0}, {0, 1, 0, 1, 0}, {0, 0, 1, 0, 0}, {0, 0, 0, 0, 0},
    };

    for (int i = 0; i < 5; ++i)
    {
        for (int j = 0; j < 5; ++j)
        {
            if (predefined[i][j] != 0)
                map.getStaticMap()[i][j].addEntity(predefined[i][j]);
        }
    }

    Feet start = Tile(2, 2).toFeet(); // inside the block
    Feet goal = Tile(4, 4).toFeet();  // outside the block
    Path path = pathFinder.findPath(map, start, goal);

    EXPECT_EQ(path.size(), 1); // no path found, just the starting pos
}

TEST_F(PathFinderAStarTest, FindPath_OnlyOneCornerBlocked)
{
    uint32_t predefined[5][5] = {
        {0, 1, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0},
    };

    for (int i = 0; i < 5; ++i)
    {
        for (int j = 0; j < 5; ++j)
        {
            if (predefined[i][j] != 0)
                map.getStaticMap()[i][j].addEntity(predefined[i][j]);
        }
    }

    Feet start = Tile(0, 0).toFeet();
    Feet goal = Tile(4, 4).toFeet();
    Path path = pathFinder.findPath(map, start, goal);

    ASSERT_TRUE(path.size() > 1);
}

TEST_F(PathFinderAStarTest, FindPath_StraightLinesUnblocked)
{
    uint32_t predefined[5][5] = {
        {0, 0, 0, 0, 0}, {0, 1, 0, 1, 0}, {0, 0, 0, 0, 0}, {0, 1, 0, 1, 0}, {0, 0, 0, 0, 0},
    };

    for (int i = 0; i < 5; ++i)
    {
        for (int j = 0; j < 5; ++j)
        {
            if (predefined[i][j] != 0)
                map.getStaticMap()[i][j].addEntity(predefined[i][j]);
        }
    }

    Feet start = Tile(2, 2).toFeet(); // inside the block
    Feet goal = Tile(4, 4).toFeet();  // outside the block
    Path path = pathFinder.findPath(map, start, goal);

    ASSERT_FALSE(path.empty());
}

TEST_F(PathFinderAStarTest, FindPath_NoPathAvailable)
{
    // Block the goal completely
    map.getStaticMap()[3][3].addEntity(1);
    map.getStaticMap()[3][4].addEntity(1);
    map.getStaticMap()[4][3].addEntity(1);

    Feet start = Tile(0, 0).toFeet();
    Feet goal = Tile(4, 4).toFeet();
    Path path = pathFinder.findPath(map, start, goal);

    EXPECT_TRUE(path.size() > 1);                          // Closer to target
    EXPECT_NE(path[path.size() - 1].toTile(), Tile(4, 4)); // but couldn't reach target
}

TEST_F(PathFinderAStarTest, FindPath_StartEqualsGoal)
{
    Feet start = Tile(2, 2).toFeet();
    Feet goal = Tile(2, 2).toFeet();
    Path path = pathFinder.findPath(map, start, goal);

    ASSERT_EQ(path.size(), 1);
    EXPECT_EQ(path.front().toTile(), start.toTile());
    EXPECT_EQ(path.back().toTile(), goal.toTile());
}

TEST_F(PathFinderAStarTest, FindPath_LargeMap_StraightLine)
{
    // Initialize a 50x50 map with no obstacles
    map.init(50, 50);

    Feet start = Tile(0, 0).toFeet();
    Feet goal = Tile(49, 0).toFeet();
    Path path = pathFinder.findPath(map, start, goal);

    ASSERT_FALSE(path.empty());
    EXPECT_EQ(path.front().toTile(), start.toTile());
    EXPECT_EQ(path.back().toTile(), goal.toTile());
}

TEST_F(PathFinderAStarTest, FindPath_LargeMap_WithObstacles)
{
    // Initialize a 50x50 map with a diagonal wall of obstacles
    map.init(50, 50);

    for (int i = 0; i < 50; ++i)
    {
        for (int j = 0; j < 50; ++j)
        {
            if (i == j)
                map.getStaticMap()[i][j].addEntity(1);
        }
    }

    map.getStaticMap()[0][0].removeAllEntities();
    map.getStaticMap()[49][49].removeAllEntities();
    Feet start = Tile(0, 49).toFeet();
    Feet goal = Tile(49, 0).toFeet();
    Path path = pathFinder.findPath(map, start, goal);

    ASSERT_FALSE(path.empty());
    EXPECT_EQ(path.front().toTile(), start.toTile());
    EXPECT_EQ(path.back().toTile(), goal.toTile());

    // Ensure the path avoids obstacles
    for (const Feet& pos : path)
    {
        auto tile = pos.toTile();
        EXPECT_TRUE(map.getStaticMap()[tile.y][tile.x].isOccupied() == false)
            << "Path crosses an obstacle at " << tile.x << ", " << tile.y;
    }
}

TEST_F(PathFinderAStarTest, FindPath_LargeMap_TargetBlocked)
{
    // Initialize a 50x50 map with a completely blocked goal
    map.init(50, 50);

    // Block the goal completely
    map.getStaticMap()[48][48].addEntity(1);
    map.getStaticMap()[48][49].addEntity(1);
    map.getStaticMap()[49][48].addEntity(1);
    map.getStaticMap()[49][49].addEntity(1);

    Feet start = Tile(0, 0).toFeet();
    Feet goal = Tile(49, 49).toFeet();
    Path path = pathFinder.findPath(map, start, goal);

    EXPECT_TRUE(path.size() > 40);                           // Path got closer to the target
    EXPECT_NE(path[path.size() - 1].toTile(), Tile(49, 49)); // but not exactly the target
}

TEST_F(PathFinderAStarTest, FindPath_LargeMap_StartEqualsGoal)
{
    // Initialize a 50x50 map with no obstacles
    map.init(50, 50);

    Feet start = Tile(25, 25).toFeet();
    Feet goal = Tile(25, 25).toFeet();
    Path path = pathFinder.findPath(map, start, goal);

    ASSERT_EQ(path.size(), 1);
    EXPECT_EQ(path.front().toTile(), start.toTile());
    EXPECT_EQ(path.back().toTile(), goal.toTile());
}
} // namespace ion