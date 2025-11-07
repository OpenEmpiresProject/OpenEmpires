#include "Tile.h"
#include "utils/Constants.h"
#include "ZOrderStrategyByTiles.h"
#include <gtest/gtest.h>
#include "Settings.h"
#include "ServiceRegistry.h"
#include "Flat2DArray.h"

namespace core
{

class ZOrderStrategyByTilesTest : public ::testing::Test
{
  protected:
    Ref<ZOrderStrategyByTiles> strategy;
    Ref<Coordinates> coordinates;
    Ref<Settings> settings;
    std::list<Ref<CompRendering>> createdEntities;
    uint32_t nextEntityId = 0;

    void SetUp() override
    {
        settings = std::make_shared<Settings>();
        settings->setWorldSizeType(WorldSizeType::TEST);
        settings->setWindowDimensions(1366, 768);


        ServiceRegistry::getInstance().registerService(settings);

        coordinates = CreateRef<Coordinates>(settings);

        strategy = CreateRef<ZOrderStrategyByTiles>();

        // clang-format off
        // Initialize a simple 5x5 map for testing
        //uint32_t predefined[5][5] = {
        //    {0, 0, 0, 0, 0}, {0, 1, 1, 1, 0}, {0, 0, 0, 1, 0}, {0, 1, 0, 0, 0}, {0, 0, 0, 0, 0},
        //};
        // clang-format on
    }

    CompRendering& createEntity(const Feet& pos, const Size& landSize)
    {
        auto rc = CreateRef<CompRendering>();
        createdEntities.push_back(rc);

        rc->positionInFeet = pos;
        rc->layer = GraphicLayer::ENTITIES;
        rc->entityID = nextEntityId++;
        rc->landSize = landSize;
        rc->srcRect.w = 100;
        rc->srcRect.h = 100;

        return *rc;
    }

    CompRendering& createBuilding(const Tile& anchor, const Size& landSize)
    {
        float dx = (float)landSize.width / 2 * Constants::FEET_PER_TILE;
        float dy = (float) landSize.height / 2 * Constants::FEET_PER_TILE;
        Tile center = anchor + Tile(1, 1);
        return createEntity(center.toFeet() - Feet(dx, dy), landSize);
    }

    CompRendering& createUnit(const Feet& pos)
    {
        return createEntity(pos, Size(0, 0));
    }
};


TEST_F(ZOrderStrategyByTilesTest, zOrder_NoEntities)
{
    auto order = strategy->zOrder(*coordinates);

    EXPECT_TRUE(order.empty());
}

/*
       0   1   2   3   4   5   6   7   8   9
     ┌───┬───┬───┬───┬───┬───┬───┬───┬───┬───┐
  0  │ T │ T │ T │ T │ T │ T │ T │ T │ T │ T │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  1  │ T │ T │ T │ T │ T │ T │ T │ T │ T │ T │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  2  │ T │ T │ T │ T │ T │ T │ T │ T │ T │ T │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  3  │ T │ T │ T │ T │ T │ T │ T │ T │ T │ T │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  4  │ T │ T │ T │ T │ T │ T │ T │ T │ T │ T │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  5  │ T │ T │ T │ T │ T │ T │ T │ T │ T │ T │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  6  │ T │ T │ T │ T │ T │ T │ T │ T │ T │ T │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  7  │ T │ T │ T │ T │ T │ T │ T │ T │ T │ T │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  8  │ T │ T │ T │ T │ T │ T │ T │ T │ T │ T │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  9  │ T │ T │ T │ T │ T │ T │ T │ T │ T │ T │
     └───┴───┴───┴───┴───┴───┴───┴───┴───┴───┘

Expectation: All tiles sort in x then y order

*/
TEST_F(ZOrderStrategyByTilesTest, zOrder_AllTilesAndNoBuildings)
{
    // Arrange
    auto worldSize = settings->getWorldSizeInTiles();
    uint32_t entityId = 0;
    Flat2DArray<CompRendering> tiles(worldSize.width, worldSize.height);

    for (int y = 0; y < worldSize.height; ++y)
    {
        for (int x = 0; x < worldSize.width; ++x)
        {
            auto& tile = tiles.at(x, y);
            tile.positionInFeet = Tile(x, y).toFeet();
            tile.layer = GraphicLayer::GROUND;
            tile.entityID = entityId++;
            tile.srcRect.w = 100;
            tile.srcRect.h = 100;

            strategy->onUpdate(CompRendering(), tile);
        }
    }

    // Act
    auto order = strategy->zOrder(*coordinates);

    // Assert
    EXPECT_EQ(order.size(), 100);
    EXPECT_EQ(order[0]->entityID, 0);
    EXPECT_EQ(order[0]->positionInFeet.toTile(), Tile(0, 0));

    EXPECT_EQ(order[10]->entityID, 10);
    EXPECT_EQ(order[10]->positionInFeet.toTile(), Tile(0, 1));

    EXPECT_EQ(order[99]->entityID, 99);
    EXPECT_EQ(order[99]->positionInFeet.toTile(), Tile(9, 9));
}



/*
       0   1   2   3   4   5   6   7   8   9
     ┌───┬───┬───┬───┬───┬───┬───┬───┬───┬───┐
  0  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  1  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  2  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  3  │   │   │   │ B │ B │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  4  │   │   │   │ B │ B │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  5  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  6  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  7  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  8  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  9  │   │   │   │   │   │   │   │   │   │   │
     └───┴───┴───┴───┴───┴───┴───┴───┴───┴───┘

Expectation: B

*/
TEST_F(ZOrderStrategyByTilesTest, zOrder_SingleBuildingAtMiddle)
{
    // Arrange
    auto& building = createBuilding(Tile(4, 4), Size(2, 2));
    strategy->onUpdate(CompRendering(), building);

    // Act
    auto order = strategy->zOrder(*coordinates);

    // Assert
    EXPECT_EQ(order.size(), 1);
    EXPECT_EQ(order[0]->entityID, building.entityID);
    EXPECT_EQ(order[0]->positionInFeet.toTile(), Tile(4, 4));
}

/*
       0   1   2   3   4   5   6   7   8   9
     ┌───┬───┬───┬───┬───┬───┬───┬───┬───┬───┐
  0  │ B │ B │ B │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  1  │ B │ B │ B │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  2  │ B │ B │ B │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  3  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  4  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  5  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  6  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  7  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  8  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  9  │   │   │   │   │   │   │   │   │   │   │
     └───┴───┴───┴───┴───┴───┴───┴───┴───┴───┘

Expectation: B; no crashes with map edges

*/
TEST_F(ZOrderStrategyByTilesTest, zOrder_SingleBuildingAtLeftTopCorner)
{
    // Arrange
    auto& building = createBuilding(Tile(2, 2), Size(3, 3));
    strategy->onUpdate(CompRendering(), building);

    // Act
    auto order = strategy->zOrder(*coordinates);

    // Assert
    EXPECT_EQ(order.size(), 1);
    EXPECT_EQ(order[0]->entityID, building.entityID);
    // Renderer use building center as the position, but not the simulator side bottom corner
    EXPECT_EQ(order[0]->positionInFeet.toTile(), Tile(1, 1));
}

/*
       0   1   2   3   4   5   6   7   8   9
     ┌───┬───┬───┬───┬───┬───┬───┬───┬───┬───┐
  0  │   │   │   │   │   │   │ B │ B │ B │ B │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  1  │   │   │   │   │   │   │ B │ B │ B │ B │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  2  │   │   │   │   │   │   │ B │ B │ B │ B │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  3  │   │   │   │   │   │   │ B │ B │ B │ B │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  4  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  5  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  6  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  7  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  8  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  9  │   │   │   │   │   │   │   │   │   │   │
     └───┴───┴───┴───┴───┴───┴───┴───┴───┴───┘

Expectation: B; no crashes with map edges

*/
TEST_F(ZOrderStrategyByTilesTest, zOrder_SingleBuildingAtRightTopCorner)
{
    // Arrange
    auto& building = createBuilding(Tile(9, 3), Size(4, 4));
    strategy->onUpdate(CompRendering(), building);

    // Act
    auto order = strategy->zOrder(*coordinates);

    // Assert
    EXPECT_EQ(order.size(), 1);
    EXPECT_EQ(order[0]->entityID, building.entityID);
    EXPECT_EQ(order[0]->positionInFeet.toTile(), Tile(8, 2));
}

/*
       0   1   2   3   4   5   6   7   8   9
     ┌───┬───┬───┬───┬───┬───┬───┬───┬───┬───┐
  0  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  1  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  2  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  3  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  4  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  5  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  6  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  7  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  8  │ B │ B │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  9  │ B │ B │   │   │   │   │   │   │   │   │
     └───┴───┴───┴───┴───┴───┴───┴───┴───┴───┘

Expectation: B; no crashes with map edges

*/
TEST_F(ZOrderStrategyByTilesTest, zOrder_SingleBuildingAtLeftBottomCorner)
{
    // Arrange
    auto& building = createBuilding(Tile(1, 9), Size(2, 2));
    strategy->onUpdate(CompRendering(), building);

    // Act
    auto order = strategy->zOrder(*coordinates);

    // Assert
    EXPECT_EQ(order.size(), 1);
    EXPECT_EQ(order[0]->entityID, building.entityID);
    EXPECT_EQ(order[0]->positionInFeet.toTile(), Tile(1, 9));
}

/*
       0   1   2   3   4   5   6   7   8   9
     ┌───┬───┬───┬───┬───┬───┬───┬───┬───┬───┐
  0  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  1  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  2  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  3  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  4  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  5  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  6  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  7  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  8  │   │   │   │   │   │   │   │   │ B │ B │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  9  │   │   │   │   │   │   │   │   │ B │ B │
     └───┴───┴───┴───┴───┴───┴───┴───┴───┴───┘

Expectation: B; no crashes with map edges

*/
TEST_F(ZOrderStrategyByTilesTest, zOrder_SingleBuildingAtRightBottomCorner)
{
    // Arrange
    auto& building = createBuilding(Tile(9, 9), Size(2, 2));
    strategy->onUpdate(CompRendering(), building);

    // Act
    auto order = strategy->zOrder(*coordinates);

    // Assert
    EXPECT_EQ(order.size(), 1);
    EXPECT_EQ(order[0]->entityID, building.entityID);
    EXPECT_EQ(order[0]->positionInFeet.toTile(), Tile(9, 9));
}

/*
       0   1   2   3   4   5   6   7   8   9
     ┌───┬───┬───┬───┬───┬───┬───┬───┬───┬───┐
  0  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  1  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  2  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  3  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  4  │   │   │   │   │ B │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  5  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  6  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  7  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  8  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  9  │   │   │   │   │   │   │   │   │   │   │
     └───┴───┴───┴───┴───┴───┴───┴───┴───┴───┘

Expectation: B

*/
TEST_F(ZOrderStrategyByTilesTest, zOrder_SingleBuildingSingleTile)
{
    // Arrange
    auto& building = createBuilding(Tile(4, 4), Size(1, 1));
    strategy->onUpdate(CompRendering(), building);

    // Act
    auto order = strategy->zOrder(*coordinates);

    // Assert
    EXPECT_EQ(order.size(), 1);
    EXPECT_EQ(order[0]->entityID, building.entityID);
    EXPECT_EQ(order[0]->positionInFeet.toTile(), Tile(4, 4));
}


/*
       0   1   2   3   4   5   6   7   8   9
     ┌───┬───┬───┬───┬───┬───┬───┬───┬───┬───┐
  0  │   │   │ B1│ B2│   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  1  │   │ B3│ B4│   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  2  │ B5│   │   │ B6│   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  3  │   │ B7│   │ B8│   │ B9│   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  4  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  5  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  6  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  7  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  8  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  9  │   │   │   │   │   │   │   │   │   │   │
     └───┴───┴───┴───┴───┴───┴───┴───┴───┴───┘

Expectation: B1, B2, B2, B4, B5, B6, B7, B8, B9

*/
TEST_F(ZOrderStrategyByTilesTest, zOrder_AllSingleTileBuildings)
{
    // Arrange
    auto& b1 = createBuilding(Tile(2, 0), Size(1, 1));
    auto& b2 = createBuilding(Tile(3, 0), Size(1, 1));
    auto& b3 = createBuilding(Tile(1, 1), Size(1, 1));
    auto& b4 = createBuilding(Tile(2, 1), Size(1, 1));
    auto& b5 = createBuilding(Tile(0, 2), Size(1, 1));
    auto& b6 = createBuilding(Tile(3, 2), Size(1, 1));
    auto& b7 = createBuilding(Tile(1, 3), Size(1, 1));
    auto& b8 = createBuilding(Tile(3, 3), Size(1, 1));
    auto& b9 = createBuilding(Tile(5, 3), Size(1, 1));

    strategy->onUpdate(CompRendering(), b1);
    strategy->onUpdate(CompRendering(), b2);
    strategy->onUpdate(CompRendering(), b3);
    strategy->onUpdate(CompRendering(), b4);
    strategy->onUpdate(CompRendering(), b5);
    strategy->onUpdate(CompRendering(), b6);
    strategy->onUpdate(CompRendering(), b7);
    strategy->onUpdate(CompRendering(), b8);
    strategy->onUpdate(CompRendering(), b9);

    // Act
    auto order = strategy->zOrder(*coordinates);

    // Assert
    EXPECT_EQ(order.size(), 9);
    EXPECT_EQ(order[0]->entityID, b1.entityID);
    EXPECT_EQ(order[1]->entityID, b2.entityID);
    EXPECT_EQ(order[2]->entityID, b3.entityID);
    EXPECT_EQ(order[3]->entityID, b4.entityID);
    EXPECT_EQ(order[4]->entityID, b5.entityID);
    EXPECT_EQ(order[5]->entityID, b6.entityID);
    EXPECT_EQ(order[6]->entityID, b7.entityID);
    EXPECT_EQ(order[7]->entityID, b8.entityID);
    EXPECT_EQ(order[8]->entityID, b9.entityID);
}
/*
       0   1   2   3   4   5   6   7   8   9
     ┌───┬───┬───┬───┬───┬───┬───┬───┬───┬───┐
  0  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  1  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  2  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  3  │   │   │   │   │ B │ B │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  4  │   │   │   │ U │ B │ B │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  5  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  6  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  7  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  8  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  9  │   │   │   │   │   │   │   │   │   │   │
     └───┴───┴───┴───┴───┴───┴───┴───┴───┴───┘

Expectation: U, B

*/
TEST_F(ZOrderStrategyByTilesTest, zOrder_SingleBuildingWithUnitAtLeftSide)
{
    // Arrange
    auto& building = createBuilding(Tile(5, 4), Size(2, 2));
    auto& unit = createUnit(Tile(3, 4).centerInFeet());
    strategy->onUpdate(CompRendering(), building);
    strategy->onUpdate(CompRendering(), unit);

    // Act
    auto order = strategy->zOrder(*coordinates);

    // Assert
    EXPECT_EQ(order.size(), 2);
    EXPECT_EQ(order[0]->entityID, unit.entityID);
    EXPECT_EQ(order[1]->entityID, building.entityID);
    EXPECT_NE(building.entityID, unit.entityID);
}


/*
       0   1   2   3   4   5   6   7   8   9
     ┌───┬───┬───┬───┬───┬───┬───┬───┬───┬───┐
  0  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  1  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  2  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  3  │   │   │   │   │ B │ B │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  4  │   │   │   │   │ B │ B │ U │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  5  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  6  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  7  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  8  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  9  │   │   │   │   │   │   │   │   │   │   │
     └───┴───┴───┴───┴───┴───┴───┴───┴───┴───┘

Expectation: B, U

*/
TEST_F(ZOrderStrategyByTilesTest, zOrder_SingleBuildingWithUnitAtRightSide)
{
    // Arrange
    auto& building = createBuilding(Tile(5, 4), Size(2, 2));
    auto& unit = createUnit(Tile(6, 4).centerInFeet());
    strategy->onUpdate(CompRendering(), building);
    strategy->onUpdate(CompRendering(), unit);

    // Act
    auto order = strategy->zOrder(*coordinates);

    // Assert
    EXPECT_EQ(order.size(), 2);
    EXPECT_EQ(order[0]->entityID, building.entityID);
    EXPECT_EQ(order[1]->entityID, unit.entityID);
    EXPECT_NE(building.entityID, unit.entityID);
}


/*
       0   1   2   3   4   5   6   7   8   9
     ┌───┬───┬───┬───┬───┬───┬───┬───┬───┬───┐
  0  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  1  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  2  │   │   │   │   │   │ U │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  3  │   │   │   │   │ B │ B │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  4  │   │   │   │   │ B │ B │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  5  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  6  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  7  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  8  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  9  │   │   │   │   │   │   │   │   │   │   │
     └───┴───┴───┴───┴───┴───┴───┴───┴───┴───┘

Expectation: U, B

*/
TEST_F(ZOrderStrategyByTilesTest, zOrder_SingleBuildingWithUnitAtTopSide)
{
    // Arrange
    auto& building = createBuilding(Tile(5, 4), Size(2, 2));
    auto& unit = createUnit(Tile(5, 2).centerInFeet());
    strategy->onUpdate(CompRendering(), building);
    strategy->onUpdate(CompRendering(), unit);

    // Act
    auto order = strategy->zOrder(*coordinates);

    // Assert
    EXPECT_EQ(order.size(), 2);
    EXPECT_EQ(order[0]->entityID, unit.entityID);
    EXPECT_EQ(order[1]->entityID, building.entityID);
    EXPECT_NE(building.entityID, unit.entityID);
}

/*
       0   1   2   3   4   5   6   7   8   9
     ┌───┬───┬───┬───┬───┬───┬───┬───┬───┬───┐
  0  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  1  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  2  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  3  │   │   │   │   │ B │ B │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  4  │   │   │   │   │ B │ B │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  5  │   │   │   │   │   │ U │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  6  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  7  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  8  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  9  │   │   │   │   │   │   │   │   │   │   │
     └───┴───┴───┴───┴───┴───┴───┴───┴───┴───┘

Expectation: B, U

*/
TEST_F(ZOrderStrategyByTilesTest, zOrder_SingleBuildingWithUnitAtBottomSide)
{
    // Arrange
    auto& building = createBuilding(Tile(5, 4), Size(2, 2));
    auto& unit = createUnit(Tile(5, 5).centerInFeet());
    strategy->onUpdate(CompRendering(), building);
    strategy->onUpdate(CompRendering(), unit);

    // Act
    auto order = strategy->zOrder(*coordinates);

    // Assert
    EXPECT_EQ(order.size(), 2);
    EXPECT_EQ(order[0]->entityID, building.entityID);
    EXPECT_EQ(order[1]->entityID, unit.entityID);
    EXPECT_NE(building.entityID, unit.entityID);
}

/*
       0   1   2   3   4   5   6   7   8   9
     ┌───┬───┬───┬───┬───┬───┬───┬───┬───┬───┐
  0  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  1  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  2  │   │   │ B1│ B1│ B2│ B2│   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  3  │   │   │ B1│ B1│ B2│ B2│   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  4  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  5  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  6  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  7  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  8  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  9  │   │   │   │   │   │   │   │   │   │   │
     └───┴───┴───┴───┴───┴───┴───┴───┴───┴───┘

Expectation: B1, B2

*/
TEST_F(ZOrderStrategyByTilesTest, zOrder_TwoConsecutiveBuildingsAlongXAxis)
{
    // Arrange
    auto& b1 = createBuilding(Tile(3, 3), Size(2, 2));
    auto& b2 = createBuilding(Tile(5, 3), Size(2, 2));
    strategy->onUpdate(CompRendering(), b1);
    strategy->onUpdate(CompRendering(), b2);

    // Act
    auto order = strategy->zOrder(*coordinates);

    // Assert
    EXPECT_EQ(order.size(), 2);
    EXPECT_EQ(order[0]->entityID, b1.entityID);
    EXPECT_EQ(order[1]->entityID, b2.entityID);
    EXPECT_NE(b1.entityID, b2.entityID);
}

/*
       0   1   2   3   4   5   6   7   8   9
     ┌───┬───┬───┬───┬───┬───┬───┬───┬───┬───┐
  0  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  1  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  2  │   │   │ B1│ B1│   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  3  │   │   │ B1│ B1│   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  4  │   │   │ B2│ B2│   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  5  │   │   │ B2│ B2│   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  6  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  7  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  8  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  9  │   │   │   │   │   │   │   │   │   │   │
     └───┴───┴───┴───┴───┴───┴───┴───┴───┴───┘

Expectation: B1, B2

*/
TEST_F(ZOrderStrategyByTilesTest, zOrder_TwoConsecutiveBuildingsAlongYAxis)
{
    // Arrange
    auto& b1 = createBuilding(Tile(3, 3), Size(2, 2));
    auto& b2 = createBuilding(Tile(3, 5), Size(2, 2));
    strategy->onUpdate(CompRendering(), b1);
    strategy->onUpdate(CompRendering(), b2);

    // Act
    auto order = strategy->zOrder(*coordinates);

    // Assert
    EXPECT_EQ(order.size(), 2);
    EXPECT_EQ(order[0]->entityID, b1.entityID);
    EXPECT_EQ(order[1]->entityID, b2.entityID);
    EXPECT_NE(b1.entityID, b2.entityID);
}

/*
       0   1   2   3   4   5   6   7   8   9
     ┌───┬───┬───┬───┬───┬───┬───┬───┬───┬───┐
  0  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  1  │   │   │   │   │   │ B1│ B1│ B1│   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  2  │   │   │   │   │   │ B1│ B1│ B1│   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  3  │   │   │ B2│ B2│   │ B1│ B1│ B1│   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  4  │   │   │ B2│ B2│   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  5  │ B3│   │ B2│ B2│   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  6  │ B3│   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  7  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  8  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  9  │   │   │   │   │   │   │   │   │   │   │
     └───┴───┴───┴───┴───┴───┴───┴───┴───┴───┘

Expectation: B3, B2, B1

*/
TEST_F(ZOrderStrategyByTilesTest, zOrder_MultipleNearbyBuildings)
{
    // Arrange
    auto& b1 = createBuilding(Tile(7, 3), Size(3, 3));
    auto& b2 = createBuilding(Tile(3, 5), Size(2, 3));
    auto& b3 = createBuilding(Tile(0, 6), Size(1, 2));
    strategy->onUpdate(CompRendering(), b1);
    strategy->onUpdate(CompRendering(), b2);
    strategy->onUpdate(CompRendering(), b3);

    // Act
    auto order = strategy->zOrder(*coordinates);

    // Assert
    EXPECT_EQ(order.size(), 3);
    EXPECT_EQ(order[0]->entityID, b3.entityID);
    EXPECT_EQ(order[1]->entityID, b2.entityID);
    EXPECT_EQ(order[2]->entityID, b1.entityID);
    EXPECT_NE(b1.entityID, b2.entityID);
    EXPECT_NE(b2.entityID, b3.entityID);
}

/*
       0   1   2   3   4   5   6   7   8   9
     ┌───┬───┬───┬───┬───┬───┬───┬───┬───┬───┐
  0  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  1  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  2  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  3  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  4  │   │   │   │   │ B1│ B1│ B1│   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  5  │   │   │   │ B2│ B1│ B1│ B1│   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  6  │   │   │   │   │ B1│ B1│ B1│   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  7  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  8  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  9  │   │   │   │   │   │   │   │   │   │   │
     └───┴───┴───┴───┴───┴───┴───┴───┴───┴───┘

Expectation: B2, B1

*/
TEST_F(ZOrderStrategyByTilesTest, zOrder_OneHugeBuildingWithTinyBuilding)
{
    // Arrange
    auto& b1 = createBuilding(Tile(6, 6), Size(3, 3));
    auto& b2 = createBuilding(Tile(3, 5), Size(1, 1));
    strategy->onUpdate(CompRendering(), b1);
    strategy->onUpdate(CompRendering(), b2);

    // Act
    auto order = strategy->zOrder(*coordinates);

    // Assert
    EXPECT_EQ(order.size(), 2);
    EXPECT_EQ(order[0]->entityID, b2.entityID);
    EXPECT_EQ(order[1]->entityID, b1.entityID);
    EXPECT_NE(b1.entityID, b2.entityID);
}

/*
       0   1   2   3   4   5   6   7   8   9
     ┌───┬───┬───┬───┬───┬───┬───┬───┬───┬───┐
  0  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  1  │   │ B │ B │ B │ B │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  2  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  3  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  4  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  5  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  6  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  7  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  8  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  9  │   │   │   │   │   │   │   │   │   │   │
     └───┴───┴───┴───┴───┴───┴───┴───┴───┴───┘

Expectation: B

*/
TEST_F(ZOrderStrategyByTilesTest, zOrder_IrregularSizeBuilding)
{
    // Arrange
    auto& b1 = createBuilding(Tile(4, 1), Size(4, 1));
    strategy->onUpdate(CompRendering(), b1);

    // Act
    auto order = strategy->zOrder(*coordinates);

    // Assert
    EXPECT_EQ(order.size(), 1);
    EXPECT_EQ(order[0]->entityID, b1.entityID);
}


/*
       0   1   2   3   4   5   6   7   8   9
     ┌───┬───┬───┬───┬───┬───┬───┬───┬───┬───┐
  0  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  1  │   │U,V│   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  2  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  3  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  4  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  5  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  6  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  7  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  8  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  9  │   │   │   │   │   │   │   │   │   │   │
     └───┴───┴───┴───┴───┴───┴───┴───┴───┴───┘

Note: U (to north) and V (to south) both are units
Expectation: U, V

*/
TEST_F(ZOrderStrategyByTilesTest, zOrder_MultipleUnitsSameTile)
{
    // Arrange
    auto& u = createUnit(Tile(1, 1).centerInFeet() - Feet(50, 50));
    auto& v = createUnit(Tile(1, 1).centerInFeet());
    strategy->onUpdate(CompRendering(), v); // Adding south unit first to break the adding order
    strategy->onUpdate(CompRendering(), u);

    // Act
    auto order = strategy->zOrder(*coordinates);

    // Assert
    EXPECT_EQ(order.size(), 2);
    EXPECT_EQ(order[0]->entityID, u.entityID);
    EXPECT_EQ(order[1]->entityID, v.entityID);
    EXPECT_NE(u.entityID, v.entityID);
}

/*
       0   1   2   3   4   5   6   7   8   9
     ┌───┬───┬───┬───┬───┬───┬───┬───┬───┬───┐
  0  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  1  │   │ U │ V │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  2  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  3  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  4  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  5  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  6  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  7  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  8  │   │   │   │   │   │   │   │   │   │   │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  9  │   │   │   │   │   │   │   │   │   │   │
     └───┴───┴───┴───┴───┴───┴───┴───┴───┴───┘

Expectation: U, V

*/
TEST_F(ZOrderStrategyByTilesTest, zOrder_MultipleUnitsDifferentTile)
{
    // Arrange
    auto& u = createUnit(Tile(1, 1).centerInFeet());
    auto& v = createUnit(Tile(2, 1).centerInFeet());
    strategy->onUpdate(CompRendering(), v); // Adding east unit first to break the adding order
    strategy->onUpdate(CompRendering(), u);

    // Act
    auto order = strategy->zOrder(*coordinates);

    // Assert
    EXPECT_EQ(order.size(), 2);
    EXPECT_EQ(order[0]->entityID, u.entityID);
    EXPECT_EQ(order[1]->entityID, v.entityID);
    EXPECT_NE(u.entityID, v.entityID);
}

TEST_F(ZOrderStrategyByTilesTest, zOrder_MovingUnit)
{
    // Arrange
    auto& u = createUnit(Tile(1, 1).centerInFeet());
    auto& v = createUnit(Tile(2, 1).centerInFeet());
    strategy->onUpdate(CompRendering(), v); // Adding east unit first to break the adding order
    strategy->onUpdate(CompRendering(), u);

    CompRendering uBefore = u;

    // Move to right side of v
    u.positionInFeet = Tile(3, 1).centerInFeet();
    strategy->onUpdate(uBefore, u);

    // Act
    auto order = strategy->zOrder(*coordinates);

    // Assert
    EXPECT_EQ(order.size(), 2);
    EXPECT_EQ(order[0]->entityID, v.entityID);
    EXPECT_EQ(order[1]->entityID, u.entityID);
    EXPECT_NE(u.entityID, v.entityID);
}

TEST_F(ZOrderStrategyByTilesTest, zOrder_SkippingDisabledOrDestroyed)
{
    // Arrange
    auto& b1 = createBuilding(Tile(6, 6), Size(1, 1));
    b1.isDestroyed = true;

    auto& b2 = createBuilding(Tile(3, 3), Size(1, 1));
    auto& b3 = createBuilding(Tile(5, 5), Size(1, 1));
    b3.isEnabled = false;

    strategy->onUpdate(CompRendering(), b1);
    strategy->onUpdate(CompRendering(), b2);
    strategy->onUpdate(CompRendering(), b3);

    // Act
    auto order = strategy->zOrder(*coordinates);

    // Assert
    EXPECT_EQ(order.size(), 1);
    EXPECT_EQ(order[0]->entityID, b2.entityID);
}

TEST_F(ZOrderStrategyByTilesTest, zOrder_HandleUIItems)
{
    // Arrange
    auto& building = createBuilding(Tile(6, 6), Size(1, 1));
    strategy->onUpdate(CompRendering(), building);

    auto& ui1 = createEntity(Feet(), Size());
    ui1.layer = GraphicLayer::UI;
    ui1.positionInFeet = Feet::null;
    ui1.positionInScreenUnits = Vec2(0, 100);
    strategy->onUpdate(CompRendering(), ui1);

    auto& ui2 = createEntity(Feet(), Size());
    ui2.layer = GraphicLayer::UI;
    ui2.positionInFeet = Feet::null;
    ui2.positionInScreenUnits = Vec2(0, 50);
    strategy->onUpdate(CompRendering(), ui2);

    // Act
    auto order = strategy->zOrder(*coordinates);

    // Assert
    EXPECT_EQ(order.size(), 3);
    EXPECT_EQ(order[0]->entityID, building.entityID);
    EXPECT_EQ(order[1]->entityID, ui2.entityID);
    EXPECT_EQ(order[2]->entityID, ui1.entityID);
}

TEST_F(ZOrderStrategyByTilesTest, zOrder_MovingUIElements)
{
    // Arrange
    auto& ui1 = createEntity(Feet(), Size());
    ui1.layer = GraphicLayer::UI;
    ui1.positionInFeet = Feet::null;
    ui1.positionInScreenUnits = Vec2(0, 100);
    strategy->onUpdate(CompRendering(), ui1);
    CompRendering ui1Before = ui1;

    auto& ui2 = createEntity(Feet(), Size());
    ui2.layer = GraphicLayer::UI;
    ui2.positionInFeet = Feet::null;
    ui2.positionInScreenUnits = Vec2(0, 50);
    strategy->onUpdate(CompRendering(), ui2);

    // ui1 has moved up
    ui1.positionInScreenUnits = Vec2(0, 25);
    strategy->onUpdate(ui1Before, ui1);

    // Act
    auto order = strategy->zOrder(*coordinates);

    // Assert
    EXPECT_EQ(order.size(), 2);
    EXPECT_EQ(order[0]->entityID, ui1.entityID);
    EXPECT_EQ(order[1]->entityID, ui2.entityID);
}


/*
       0   1   2   3   4   5   6   7   8   9
     ┌───┬───┬───┬───┬───┬───┬───┬───┬───┬───┐
  0  │ T │ T │ T │ T │ T │ T │ T │ T │ T │ T │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  1  │ T │B,T│B,T│ T │ T │ T │ T │ T │ T │ T │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  2  │ T │B,T│B,T│ T │ T │ T │ T │ T │ T │ T │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  3  │ T │ T │ T │ T │ T │ T │ T │ T │ T │ T │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  4  │ T │ T │ T │ T │ T │ T │ T │ T │ T │ T │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  5  │ T │ T │ T │ T │ T │ T │ T │ T │ T │ T │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  6  │ T │ T │ T │ T │ T │ T │ T │ T │ T │ T │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  7  │ T │ T │ T │ T │ T │ T │ T │ T │ T │ T │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  8  │ T │ T │ T │ T │ T │ T │ T │ T │ T │ T │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  9  │ T │ T │ T │ T │ T │ T │ T │ T │ T │ T │
     └───┴───┴───┴───┴───┴───┴───┴───┴───┴───┘

Expectation: All tiles sort in x then y order

*/
TEST_F(ZOrderStrategyByTilesTest, zOrder_BuildingOnGroundTile)
{
    // Arrange
    auto worldSize = settings->getWorldSizeInTiles();
    uint32_t entityId = 0;
    Flat2DArray<CompRendering> tiles(worldSize.width, worldSize.height);

    for (int y = 0; y < worldSize.height; ++y)
    {
        for (int x = 0; x < worldSize.width; ++x)
        {
            auto& tile = tiles.at(x, y);
            tile.positionInFeet = Tile(x, y).toFeet();
            tile.layer = GraphicLayer::GROUND;
            tile.entityID = entityId++;
            tile.srcRect.w = 100;
            tile.srcRect.h = 100;

            strategy->onUpdate(CompRendering(), tile);
        }
    }

    auto& building = createBuilding(Tile(2, 2), Size(2, 2));
    building.entityID = entityId++;
    strategy->onUpdate(CompRendering(), building);

    // Act
    auto order = strategy->zOrder(*coordinates);

    // Assert
    EXPECT_EQ(order.size(), 101);
    EXPECT_EQ(order[0]->entityID, 0);
    EXPECT_EQ(order[0]->positionInFeet.toTile(), Tile(0, 0));

    EXPECT_EQ(order[10]->entityID, 10);
    EXPECT_EQ(order[10]->positionInFeet.toTile(), Tile(0, 1));

    EXPECT_EQ(order[99]->entityID, 99);
    EXPECT_EQ(order[99]->positionInFeet.toTile(), Tile(9, 9));

    EXPECT_EQ(order[100]->entityID, building.entityID);
    EXPECT_EQ(order[100]->positionInFeet.toTile(), Tile(2, 2));
}


/*
       0   1   2   3   4   5   6   7   8   9
     ┌───┬───┬───┬───┬───┬───┬───┬───┬───┬───┐
  0  │ T │ T │ T │ T │ T │ T │ T │ T │ T │ T │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  1  │ T │ T │ T │ T │ T │ T │ T │ T │ T │ T │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  2  │ T │ T │ T │ T │ T │ T │ T │ T │ T │ T │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  3  │ T │ T │ T │ T │ T │ T │ T │ T │ T │ T │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  4  │ T │ T │ T │ T │ T │ T │ T │ T │ T │ T │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  5  │ T │ T │ T │ T │ T │ T │ T │ T │ T │ T │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  6  │ T │ T │ T │ T │ T │ T │ T │ T │ T │ T │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  7  │ T │ T │ T │ T │ T │ T │ T │ T │ T │ T │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  8  │ T │ T │ T │ T │ T │ T │ T │ T │ T │ T │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  9  │ T │ T │ T │ T │ T │ T │ T │ T │ T │ T │
     └───┴───┴───┴───┴───┴───┴───┴───┴───┴───┘

Expectation: All tiles sort in x then y order

*/
TEST_F(ZOrderStrategyByTilesTest, zOrder_AllTilesButLimitedViewport)
{
    // Arrange
    settings->setWindowDimensions(200, 200);
    auto worldSize = settings->getWorldSizeInTiles();
    uint32_t entityId = 0;
    Flat2DArray<CompRendering> tiles(worldSize.width, worldSize.height);

    for (int y = 0; y < worldSize.height; ++y)
    {
        for (int x = 0; x < worldSize.width; ++x)
        {
            auto& tile = tiles.at(x, y);
            tile.positionInFeet = Tile(x, y).toFeet();
            tile.layer = GraphicLayer::GROUND;
            tile.entityID = entityId++;
            tile.srcRect.w = 100;
            tile.srcRect.h = 100;

            strategy->onUpdate(CompRendering(), tile);
        }
    }

    // Act
    auto order = strategy->zOrder(*coordinates);

    // Assert
    // Since the texture size is mimicked to be 100x100, viewport of 200x200 can see at most 
    // 4 tiles.
    EXPECT_EQ(order.size(), 4);
}


/*
       0   1   2   3   4   5   6   7   8   9
     ┌───┬───┬───┬───┬───┬───┬───┬───┬───┬───┐
  0  │ T │ T │ T │ T │ T │ T │ T │ T │ T │ T │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  1  │ T │ T │ T │ T │ T │ T │ T │ T │ T │ T │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  2  │ T │ T │ T │ T │ T │ T │ T │ T │ T │ T │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  3  │ T │ T │ T │ T │ T │ T │ T │ T │ T │ T │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  4  │ T │ T │ T │ T │ T │ T │ T │ T │ T │ T │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  5  │ T │ T │ T │ T │ T │ T │ T │ T │ T │ T │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  6  │ T │ T │ T │ T │ T │ T │ T │ T │ T │ T │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  7  │ T │ T │ T │ T │ T │ T │ T │ T │ T │ T │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  8  │ T │ T │ T │ T │ T │ T │ T │ T │ T │ T │
     ├───┼───┼───┼───┼───┼───┼───┼───┼───┼───┤
  9  │ T │ T │ T │ T │ T │ T │ T │ T │ T │ T │
     └───┴───┴───┴───┴───┴───┴───┴───┴───┴───┘

Expectation: only the tiles inside extended viewport is ordered

*/
TEST_F(ZOrderStrategyByTilesTest, zOrder_ExtendedViewportFilteration)
{
    // Arrange
    auto worldSize = settings->getWorldSizeInTiles();
    uint32_t entityId = 0;
    Flat2DArray<CompRendering> tiles(worldSize.width, worldSize.height);

    for (int y = 0; y < worldSize.height; ++y)
    {
        for (int x = 0; x < worldSize.width; ++x)
        {
            auto& tile = tiles.at(x, y);
            tile.positionInFeet = Tile(x, y).toFeet();
            tile.layer = GraphicLayer::GROUND;
            tile.entityID = entityId++;
            tile.srcRect.w = 100;
            tile.srcRect.h = 100;

            strategy->onUpdate(CompRendering(), tile);
        }
    }
    settings->setWindowDimensions(200, 200);

    // Act
    auto order = strategy->zOrder(*coordinates);

    // Assert - not all tiles should be processed
    EXPECT_LT(order.size(), 100);
}

} // namespace core