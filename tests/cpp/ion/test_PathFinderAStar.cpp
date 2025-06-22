#include "PathFinderAStar.h"
#include <gtest/gtest.h>

using namespace ion;

class PathFinderAStarTest : public ::testing::Test {
protected:
    PathFinderAStar pathFinder;
    TileMap map;

    void SetUp() override {
        map.init(5, 5);

        // Initialize a simple 5x5 map for testing
        uint32_t predefined[5][5] = {
            {0, 0, 0, 0, 0},
            {0, 1, 1, 1, 0},
            {0, 0, 0, 1, 0},
            {0, 1, 0, 0, 0},
            {0, 0, 0, 0, 0},
        };

        for (int i = 0; i < 5; ++i) {
            for (int j = 0; j < 5; ++j) {
                map.getStaticMap()[i][j].addEntity(predefined[i][j]);
            }
        }
    }
};

TEST_F(PathFinderAStarTest, FindPath_StraightLine) {
    Feet start = {0, 0};
    Feet goal = {4, 0};
    Path path = pathFinder.findPath(map, start, goal);

    ASSERT_FALSE(path.empty());
    EXPECT_EQ(path.front(), start);
    EXPECT_EQ(path.back(), goal);
}

TEST_F(PathFinderAStarTest, FindPath_AroundObstacle) {
    Feet start = {0, 0};
    Feet goal = {4, 4};
    Path path = pathFinder.findPath(map, start, goal);

    ASSERT_FALSE(path.empty());
    EXPECT_EQ(path.front(), start);
    EXPECT_EQ(path.back(), goal);

    // Ensure the path avoids obstacles
    for (const Feet& pos : path) {
        EXPECT_TRUE(map.getStaticMap()[pos.x][pos.y].isOccupied() == false) << "Path crosses an obstacle at " << pos.x << ", " << pos.y;
    }
}

TEST_F(PathFinderAStarTest, FindPath_CornersBlocked) {
    uint32_t predefined[5][5] = {
        {0, 0, 0, 0, 0},
        {0, 0, 1, 0, 0},
        {0, 1, 0, 1, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 0, 0, 0},
    };

    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 5; ++j) {
            map.getStaticMap()[i][j].addEntity(predefined[i][j]);
        }
    }

    Feet start = {2, 2}; // inside the block
    Feet goal = {4, 4}; // outside the block
    Path path = pathFinder.findPath(map, start, goal);

    ASSERT_TRUE(path.empty());
}

TEST_F(PathFinderAStarTest, FindPath_OnlyOneCornerBlocked) {
    uint32_t predefined[5][5] = {
        {0, 1, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
    };

    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 5; ++j) {
            map.getStaticMap()[i][j].addEntity(predefined[i][j]);
        }
    }

    Feet start = {0, 0};
    Feet goal = {4, 4};
    Path path = pathFinder.findPath(map, start, goal);

    ASSERT_EQ(path.size(), 6);
}

TEST_F(PathFinderAStarTest, FindPath_StraightLinesUnblocked) {
    uint32_t predefined[5][5] = {
        {0, 0, 0, 0, 0},
        {0, 1, 0, 1, 0},
        {0, 0, 0, 0, 0},
        {0, 1, 0, 1, 0},
        {0, 0, 0, 0, 0},
    };

    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 5; ++j) {
            map.getStaticMap()[i][j].addEntity(predefined[i][j]);
        }
    }

    Feet start = {2, 2}; // inside the block
    Feet goal = {4, 4}; // outside the block
    Path path = pathFinder.findPath(map, start, goal);

    ASSERT_FALSE(path.empty());
}

TEST_F(PathFinderAStarTest, FindPath_NoPathAvailable) {
    // Block the goal completely
    map.getStaticMap()[3][3].addEntity(1);
    map.getStaticMap()[3][4].addEntity(1);
    map.getStaticMap()[4][3].addEntity(1);

    Feet start = {0, 0};
    Feet goal = {4, 4};
    Path path = pathFinder.findPath(map, start, goal);

    EXPECT_TRUE(path.empty());
}

TEST_F(PathFinderAStarTest, FindPath_StartEqualsGoal) {
    Feet start = {2, 2};
    Feet goal = {2, 2};
    Path path = pathFinder.findPath(map, start, goal);

    ASSERT_EQ(path.size(), 1);
    EXPECT_EQ(path.front(), start);
    EXPECT_EQ(path.back(), goal);
}

TEST_F(PathFinderAStarTest, FindPath_LargeMap_StraightLine) {
    // Initialize a 50x50 map with no obstacles
    map.init(50, 50);

    Feet start = {0, 0};
    Feet goal = {49, 0};
    Path path = pathFinder.findPath(map, start, goal);

    ASSERT_FALSE(path.empty());
    EXPECT_EQ(path.front(), start);
    EXPECT_EQ(path.back(), goal);
}

TEST_F(PathFinderAStarTest, FindPath_LargeMap_WithObstacles) {
    // Initialize a 50x50 map with a diagonal wall of obstacles
    map.init(50, 50);

    for (int i = 0; i < 50; ++i) {
        for (int j = 0; j < 50; ++j) {
            map.getStaticMap()[i][j].addEntity((i == j) ? 1 : 0);
        }
    }

    map.getStaticMap()[0][0].removeAllEntities(); 
    map.getStaticMap()[49][49].removeAllEntities();
    Feet start = {0, 49};
    Feet goal = {49, 0};
    Path path = pathFinder.findPath(map, start, goal);

    ASSERT_FALSE(path.empty());
    EXPECT_EQ(path.front(), start);
    EXPECT_EQ(path.back(), goal);

    // Ensure the path avoids obstacles
    for (const Feet& pos : path) {
        EXPECT_TRUE(map.getStaticMap()[pos.y][pos.x].isOccupied() == false) << "Path crosses an obstacle at " << pos.x << ", " << pos.y;
    }
}

TEST_F(PathFinderAStarTest, FindPath_LargeMap_NoPathAvailable) {
    // Initialize a 50x50 map with a completely blocked goal
    map.init(50, 50);

    // Block the goal completely
    map.getStaticMap()[48][48].addEntity(1);
    map.getStaticMap()[48][49].addEntity(1);
    map.getStaticMap()[49][48].addEntity(1);
    map.getStaticMap()[49][49].addEntity(1);

    Feet start = {0, 0};
    Feet goal = {49, 49};
    Path path = pathFinder.findPath(map, start, goal);

    EXPECT_TRUE(path.empty());
}

TEST_F(PathFinderAStarTest, FindPath_LargeMap_StartEqualsGoal) {
    // Initialize a 50x50 map with no obstacles
    map.init(50, 50);

    Feet start = {25, 25};
    Feet goal = {25, 25};
    Path path = pathFinder.findPath(map, start, goal);

    ASSERT_EQ(path.size(), 1);
    EXPECT_EQ(path.front(), start);
    EXPECT_EQ(path.back(), goal);
}