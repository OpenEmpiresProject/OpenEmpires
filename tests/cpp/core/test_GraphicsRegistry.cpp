#ifndef TEST_GRAPHICSREGISTRY_H
#define TEST_GRAPHICSREGISTRY_H

#include "GraphicsRegistry.h"
#include <gtest/gtest.h>

namespace core 
{

TEST(GraphicsIDTest, DefaultConstructor) 
{
    using namespace core;

    GraphicsID defaultID;
    EXPECT_EQ(defaultID.entityType, 0);
    EXPECT_EQ(defaultID.action, 0);
    EXPECT_EQ(defaultID.frame, 0);
    EXPECT_EQ(defaultID.direction, Direction::NONE);
    EXPECT_EQ(defaultID.entitySubType, 0);
    EXPECT_EQ(defaultID.variation, 0);
    EXPECT_EQ(defaultID.reserved, 0);
}

TEST(GraphicsIDTest, HashAndFromHash) 
{
    using namespace core;

    GraphicsID id3{6, 7, 8, 9, Direction::SOUTH};
    int64_t hashValue = id3.hash();
    GraphicsID idFromHash = GraphicsID::fromHash(hashValue);
    EXPECT_EQ(id3, idFromHash);
}


TEST(GraphicsIDTest, DuplicatedHashDueToDirection) 
{
    // This was an actual issue happened during game run. 
    GraphicsID id1{3, 0, 8, 0, Direction::SOUTH};
    GraphicsID id2{3, 0, 0, 0, Direction::NORTHWEST};

    int64_t hashValue1 = id1.hash();
    int64_t hashValue2 = id2.hash();

    EXPECT_NE(hashValue1, hashValue2);
}

TEST(GraphicsIDTest, ToString) 
{
    using namespace core;

    GraphicsID id3{6, 7, 8, 0, Direction::SOUTH};
    std::string idStr = id3.toString();
    std::cout << "GraphicsID toString: " << idStr << std::endl;
}

TEST(GraphicsIDTest, PlayerIdIsEncodedInHash)
{
    using namespace core;

    GraphicsID id;
    id.entityType = 1;
    id.entitySubType = 2;
    id.action = 3;
    id.frame = 4;
    id.direction = Direction::EAST;
    id.variation = 5;
    id.playerId = 12;
    id.reserved = 15;

    int64_t hash = id.hash();
    GraphicsID fromHash = GraphicsID::fromHash(hash);

    EXPECT_EQ(fromHash.playerId, 12);
    EXPECT_EQ(fromHash.reserved, 15);
    EXPECT_EQ(fromHash, id);
}

TEST(GraphicsIDTest, PlayerIdAffectsHash)
{
    using namespace core;

    GraphicsID id1{6, 7, 8, 9, Direction::SOUTH};
    id1.playerId = 1;

    GraphicsID id2 = id1;
    id2.playerId = 2;

    EXPECT_NE(id1.hash(), id2.hash());
    EXPECT_NE(id1, id2);
}

TEST(GraphicsIDTest, PlayerIdInToString)
{
    using namespace core;

    GraphicsID id;
    id.entityType = 1;
    id.entitySubType = 2;
    id.action = 3;
    id.frame = 4;
    id.direction = Direction::WEST;
    id.variation = 5;
    id.playerId = 6;
    id.reserved = 7;

    std::string str = id.toString();
    std::cout << "GraphicsID toString: " << str << std::endl;

    // Check for player ID and reserved fields in output
    EXPECT_NE(str.find("P6"), std::string::npos);
    EXPECT_NE(str.find("R7"), std::string::npos);
}
TEST(GraphicsIDTest, DefaultPlayerIdIsZero)
{
    using namespace core;

    GraphicsID id;
    EXPECT_EQ(id.playerId, 0);
}

TEST(GraphicsRegistryTest, RegisterAndRetrieveGraphic) 
{
    using namespace core;

    GraphicsRegistry registry;

    GraphicsID id1{1, 2, 3, 4, Direction::EAST};
    Texture entry1;
    entry1.anchor = Vec2(10, 20);

    registry.registerTexture(id1, entry1);
    const Texture& retrievedEntry = registry.getTexture(id1);
    EXPECT_EQ(retrievedEntry.anchor.x, 10);
    EXPECT_EQ(retrievedEntry.anchor.y, 20);
}

TEST(GraphicsRegistryTest, GetGraphicsCount) 
{
    using namespace core;

    GraphicsRegistry registry;

    GraphicsID id1{1, 2, 3, 4, Direction::EAST};
    Texture entry1;
    entry1.anchor = Vec2(10, 20);

    registry.registerTexture(id1, entry1);
    EXPECT_EQ(registry.getTextureCount(), 1);

    GraphicsID id2{4, 5, 6, 7, Direction::WEST};
    Texture entry2;
    entry2.anchor = Vec2(30, 40);

    registry.registerTexture(id2, entry2);
    EXPECT_EQ(registry.getTextureCount(), 2);

    const Texture& retrievedEntry2 = registry.getTexture(id2);
    EXPECT_EQ(retrievedEntry2.anchor.x, 30);
    EXPECT_EQ(retrievedEntry2.anchor.y, 40);
}

}  // namespace

#endif