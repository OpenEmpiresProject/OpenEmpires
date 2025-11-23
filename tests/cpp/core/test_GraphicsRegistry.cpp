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

TEST(GraphicsIDTest, ToString) 
{
    using namespace core;

    GraphicsID id3;
    id3.entityType = 6;
    id3.entitySubType = 7;
    std::string idStr = id3.toString();
    std::cout << "GraphicsID toString: " << idStr << std::endl;
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

    std::string str = id.toString();
    std::cout << "GraphicsID toString: " << str << std::endl;

    // Check for player ID and reserved fields in output
    EXPECT_NE(str.find("P6"), std::string::npos);
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

    GraphicsID id1;
    id1.entityType = 1;
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

    GraphicsID id1; //{1, 2, 3, 4, Direction::EAST};
    id1.entityType = 1;
    Texture entry1;
    entry1.anchor = Vec2(10, 20);

    registry.registerTexture(id1, entry1);
    EXPECT_EQ(registry.getTextureCount(), 1);

    GraphicsID id2; //{4, 5, 6, 7, Direction::WEST};
    id2.entityType = 4;
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