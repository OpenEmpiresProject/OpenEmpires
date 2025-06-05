#include "PathFinderAStar.h"
#include <gtest/gtest.h>

using namespace ion;

class PathFinderAStarTest : public ::testing::Test {
protected:
    PathFinderAStar pathFinder;
    StaticEntityMap map;

    void SetUp() override {
        // Initialize a simple 5x5 map for testing
        map.width = 5;
        map.height = 5;
        int predefined[5][5] = {
            {0, 0, 0, 0, 0},
            {0, 1, 1, 1, 0},
            {0, 0, 0, 1, 0},
            {0, 1, 0, 0, 0},
            {0, 0, 0, 0, 0},
        };

        map.map = new int*[5];
        for (int i = 0; i < 5; ++i) {
            map.map[i] = new int[5];
            for (int j = 0; j < 5; ++j) {
                map.map[i][j] = predefined[i][j];
            }
        }
    }
};

TEST_F(PathFinderAStarTest, FindPath_StraightLine) {
    Vec2d start = {0, 0};
    Vec2d goal = {4, 0};
    Path path = pathFinder.findPath(map, start, goal);

    ASSERT_FALSE(path.empty());
    EXPECT_EQ(path.front(), start);
    EXPECT_EQ(path.back(), goal);
}

TEST_F(PathFinderAStarTest, FindPath_AroundObstacle) {
    Vec2d start = {0, 0};
    Vec2d goal = {4, 4};
    Path path = pathFinder.findPath(map, start, goal);

    ASSERT_FALSE(path.empty());
    EXPECT_EQ(path.front(), start);
    EXPECT_EQ(path.back(), goal);

    // Ensure the path avoids obstacles
    for (const Vec2d& pos : path) {
        EXPECT_TRUE(map.map[pos.x][pos.y] == 0) << "Path crosses an obstacle at " << pos.x << ", " << pos.y;
    }
}

TEST_F(PathFinderAStarTest, FindPath_CornersBlocked) {
    int predefined[5][5] = {
        {0, 0, 0, 0, 0},
        {0, 0, 1, 0, 0},
        {0, 1, 0, 1, 0},
        {0, 0, 1, 0, 0},
        {0, 0, 0, 0, 0},
    };

    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 5; ++j) {
            map.map[i][j] = predefined[i][j];
        }
    }

    Vec2d start = {2, 2}; // inside the block
    Vec2d goal = {4, 4}; // outside the block
    Path path = pathFinder.findPath(map, start, goal);

    ASSERT_TRUE(path.empty());
}

TEST_F(PathFinderAStarTest, FindPath_OnlyOneCornerBlocked) {
    int predefined[5][5] = {
        {0, 1, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0},
    };

    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 5; ++j) {
            map.map[i][j] = predefined[i][j];
        }
    }

    Vec2d start = {0, 0};
    Vec2d goal = {4, 4};
    Path path = pathFinder.findPath(map, start, goal);

    ASSERT_EQ(path.size(), 6);
}

TEST_F(PathFinderAStarTest, FindPath_StraightLinesUnblocked) {
    int predefined[5][5] = {
        {0, 0, 0, 0, 0},
        {0, 1, 0, 1, 0},
        {0, 0, 0, 0, 0},
        {0, 1, 0, 1, 0},
        {0, 0, 0, 0, 0},
    };

    for (int i = 0; i < 5; ++i) {
        for (int j = 0; j < 5; ++j) {
            map.map[i][j] = predefined[i][j];
        }
    }

    Vec2d start = {2, 2}; // inside the block
    Vec2d goal = {4, 4}; // outside the block
    Path path = pathFinder.findPath(map, start, goal);

    ASSERT_FALSE(path.empty());
}

TEST_F(PathFinderAStarTest, FindPath_NoPathAvailable) {
    // Block the goal completely
    map.map[3][3] = 1;
    map.map[3][4] = 1;
    map.map[4][3] = 1;

    Vec2d start = {0, 0};
    Vec2d goal = {4, 4};
    Path path = pathFinder.findPath(map, start, goal);

    EXPECT_TRUE(path.empty());
}

TEST_F(PathFinderAStarTest, FindPath_StartEqualsGoal) {
    Vec2d start = {2, 2};
    Vec2d goal = {2, 2};
    Path path = pathFinder.findPath(map, start, goal);

    ASSERT_EQ(path.size(), 1);
    EXPECT_EQ(path.front(), start);
    EXPECT_EQ(path.back(), goal);
}

TEST_F(PathFinderAStarTest, FindPath_LargeMap_StraightLine) {
    // Initialize a 50x50 map with no obstacles
    map.width = 50;
    map.height = 50;
    map.map = new int*[50];
    for (int i = 0; i < 50; ++i) {
        map.map[i] = new int[50];
        for (int j = 0; j < 50; ++j) {
            map.map[i][j] = 0;
        }
    }

    Vec2d start = {0, 0};
    Vec2d goal = {49, 0};
    Path path = pathFinder.findPath(map, start, goal);

    ASSERT_FALSE(path.empty());
    EXPECT_EQ(path.front(), start);
    EXPECT_EQ(path.back(), goal);
}

TEST_F(PathFinderAStarTest, FindPath_LargeMap_WithObstacles) {
    // Initialize a 50x50 map with a diagonal wall of obstacles
    map.width = 50;
    map.height = 50;
    map.map = new int*[50];
    for (int i = 0; i < 50; ++i) {
        map.map[i] = new int[50];
        for (int j = 0; j < 50; ++j) {
            map.map[i][j] = (i == j) ? 1 : 0; // Diagonal wall
        }
    }
    map.map[0][0] = 0; 
    map.map[49][49] = 0;

    Vec2d start = {0, 49};
    Vec2d goal = {49, 0};
    Path path = pathFinder.findPath(map, start, goal);

    ASSERT_FALSE(path.empty());
    EXPECT_EQ(path.front(), start);
    EXPECT_EQ(path.back(), goal);

    // Ensure the path avoids obstacles
    for (const Vec2d& pos : path) {
        EXPECT_TRUE(map.map[pos.y][pos.x] == 0) << "Path crosses an obstacle at " << pos.x << ", " << pos.y;
    }
}

TEST_F(PathFinderAStarTest, FindPath_LargeMap_NoPathAvailable) {
    // Initialize a 50x50 map with a completely blocked goal
    map.width = 50;
    map.height = 50;
    map.map = new int*[50];
    for (int i = 0; i < 50; ++i) {
        map.map[i] = new int[50];
        for (int j = 0; j < 50; ++j) {
            map.map[i][j] = 0;
        }
    }

    // Block the goal completely
    map.map[48][48] = 1;
    map.map[48][49] = 1;
    map.map[49][48] = 1;
    map.map[49][49] = 1;

    Vec2d start = {0, 0};
    Vec2d goal = {49, 49};
    Path path = pathFinder.findPath(map, start, goal);

    EXPECT_TRUE(path.empty());
}

TEST_F(PathFinderAStarTest, FindPath_LargeMap_StartEqualsGoal) {
    // Initialize a 50x50 map with no obstacles
    map.width = 50;
    map.height = 50;
    map.map = new int*[50];
    for (int i = 0; i < 50; ++i) {
        map.map[i] = new int[50];
        for (int j = 0; j < 50; ++j) {
            map.map[i][j] = 0;
        }
    }

    Vec2d start = {25, 25};
    Vec2d goal = {25, 25};
    Path path = pathFinder.findPath(map, start, goal);

    ASSERT_EQ(path.size(), 1);
    EXPECT_EQ(path.front(), start);
    EXPECT_EQ(path.back(), goal);
}