#ifndef TEST_GRAPHICSREGISTRY_H
#define TEST_GRAPHICSREGISTRY_H

#include "GraphicsRegistry.h"
#include <gtest/gtest.h>

namespace ion {

TEST(GraphicsIDTest, DefaultConstructor) {
    using namespace ion;

    GraphicsID defaultID;
    EXPECT_EQ(defaultID.entityType, 0);
    EXPECT_EQ(defaultID.action, 0);
    EXPECT_EQ(defaultID.frame, 0);
    EXPECT_EQ(defaultID.direction, Direction::NONE);
    EXPECT_EQ(defaultID.entitySubType, 0);
    EXPECT_EQ(defaultID.variation, 0);
    EXPECT_EQ(defaultID.reserved, 0);
}

// TEST(GraphicsIDTest, ParameterizedConstructors) {
//     using namespace ion;

//     GraphicsID id1(1, 2);
//     EXPECT_EQ(id1.entityType, 1);
//     EXPECT_EQ(id1.action, 2);

//     GraphicsID id2(3, 4, 5);
//     EXPECT_EQ(id2.entityType, 3);
//     EXPECT_EQ(id2.action, 4);
//     EXPECT_EQ(id2.frame, 5);

//     GraphicsID id3(6, 7, 8, Direction::SOUTH);
//     EXPECT_EQ(id3.entityType, 6);
//     EXPECT_EQ(id3.action, 7);
//     EXPECT_EQ(id3.frame, 8);
//     EXPECT_EQ(id3.direction, Direction::SOUTH);
// }

TEST(GraphicsIDTest, HashAndFromHash) {
    using namespace ion;

    GraphicsID id3{6, 7, 8, 9, Direction::SOUTH};
    int64_t hashValue = id3.hash();
    GraphicsID idFromHash = GraphicsID::fromHash(hashValue);
    EXPECT_EQ(id3, idFromHash);
}


TEST(GraphicsIDTest, DuplicatedHashDueToDirection) {
    // This was an actual issue happened during game run. 
    GraphicsID id1{3, 0, 8, 0, Direction::SOUTH};
    GraphicsID id2{3, 0, 0, 0, Direction::NORTHWEST};

    int64_t hashValue1 = id1.hash();
    int64_t hashValue2 = id2.hash();

    EXPECT_NE(hashValue1, hashValue2);
}


TEST(GraphicsIDTest, ToString) {
    using namespace ion;

    GraphicsID id3{6, 7, 8, 0, Direction::SOUTH};
    std::string idStr = id3.toString();
    std::cout << "GraphicsID toString: " << idStr << std::endl;
}

TEST(GraphicsRegistryTest, RegisterAndRetrieveGraphic) {
    using namespace ion;

    GraphicsRegistry registry;

    GraphicsID id1{1, 2, 3, 4, Direction::EAST};
    Texture entry1;
    entry1.anchor = Vec2d(10, 20);

    registry.registerTexture(id1, entry1);
    const Texture& retrievedEntry = registry.getTexture(id1);
    EXPECT_EQ(retrievedEntry.anchor.x, 10);
    EXPECT_EQ(retrievedEntry.anchor.y, 20);
}

TEST(GraphicsRegistryTest, GetGraphicsCount) {
    using namespace ion;

    GraphicsRegistry registry;

    GraphicsID id1{1, 2, 3, 4, Direction::EAST};
    Texture entry1;
    entry1.anchor = Vec2d(10, 20);

    registry.registerTexture(id1, entry1);
    EXPECT_EQ(registry.getTextureCount(), 1);

    GraphicsID id2{4, 5, 6, 7, Direction::WEST};
    Texture entry2;
    entry2.anchor = Vec2d(30, 40);

    registry.registerTexture(id2, entry2);
    EXPECT_EQ(registry.getTextureCount(), 2);

    const Texture& retrievedEntry2 = registry.getTexture(id2);
    EXPECT_EQ(retrievedEntry2.anchor.x, 30);
    EXPECT_EQ(retrievedEntry2.anchor.y, 40);
}

}  // namespace

#endif