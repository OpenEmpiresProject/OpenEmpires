#include "VersionedBucketList.h"

#include <cstdint>
#include <gtest/gtest.h>
#include <string>
#include <utility>
#include <vector>


// Helper types for testing
namespace core
{

// Movable type that marks moved-from state
struct MoveTrack
{
    int value;
    bool movedFrom = false;

    MoveTrack(int v = 0) : value(v), movedFrom(false)
    {
    }
    MoveTrack(const MoveTrack& other) = default;
    MoveTrack& operator=(const MoveTrack& other) = default;

    MoveTrack(MoveTrack&& other) noexcept : value(other.value)
    {
        other.value = -1;
        other.movedFrom = true;
        // movedFrom stays false on the new object
    }

    MoveTrack& operator=(MoveTrack&& other) noexcept
    {
        if (this != &other)
        {
            value = other.value;
            other.value = -1;
            other.movedFrom = true;
        }
        return *this;
    }

    bool operator==(const MoveTrack& o) const
    {
        return value == o.value;
    }
};

// Type constructed via variadic emplace args
struct Emplaceable
{
    int a;
    int b;
    std::string s;

    Emplaceable(int aa, int bb, std::string ss) : a(aa), b(bb), s(std::move(ss))
    {
    }
    bool operator==(const Emplaceable& o) const
    {
        return a == o.a && b == o.b && s == o.s;
    }
};

TEST(VersionedBucketSorterTest, VersionIncrementBehavior)
{
    VersionedBucketList<int> sorter(3);

    uint64_t before = sorter.getVersion();
    sorter.incrementVersion();
    EXPECT_EQ(sorter.getVersion(), before + 1);

    // Multiple increments
    sorter.incrementVersion();
    sorter.incrementVersion();
    EXPECT_EQ(sorter.getVersion(), before + 3);
}

TEST(VersionedBucketSorterTest, InitiallyNoItems)
{
    VersionedBucketList<int> sorter(4);

    for (size_t i = 0; i < 4; ++i)
    {
        EXPECT_FALSE(sorter.hasItems(i));
        const auto& items = sorter.getItems(i);
        EXPECT_TRUE(items.empty());
    }
}

TEST(VersionedBucketSorterTest, EmplaceCopyAndHasItemsAndGetItems)
{
    VersionedBucketList<int> sorter(2);

    EXPECT_FALSE(sorter.hasItems(0));
    sorter.emplaceItem(0, 42); // should use emplace with Args or copy
    EXPECT_TRUE(sorter.hasItems(0));

    const auto& items0 = sorter.getItems(0);
    ASSERT_EQ(items0.size(), 1u);
    EXPECT_EQ(items0[0], 42);

    // Emplace more items to same bucket, preserve order
    sorter.emplaceItem(0, 7);
    sorter.emplaceItem(0, 13);
    const auto& items0b = sorter.getItems(0);
    ASSERT_EQ(items0b.size(), 3u);
    EXPECT_EQ(items0b[0], 42);
    EXPECT_EQ(items0b[1], 7);
    EXPECT_EQ(items0b[2], 13);

    // Other bucket still empty
    EXPECT_FALSE(sorter.hasItems(1));
    EXPECT_TRUE(sorter.getItems(1).empty());
}

TEST(VersionedBucketSorterTest, EmplaceMoveOverload)
{
    VersionedBucketList<MoveTrack> sorter(2);

    MoveTrack m(123);
    // copy via lvalue
    sorter.emplaceItem(0, m);
    EXPECT_EQ(sorter.getItems(0).size(), 1u);
    EXPECT_EQ(sorter.getItems(0)[0].value, 123);
    EXPECT_EQ(m.value, 123); // original unchanged

    // move via rvalue
    MoveTrack m2(555);
    sorter.emplaceItem(0, std::move(m2));
    ASSERT_EQ(sorter.getItems(0).size(), 2u);
    EXPECT_EQ(sorter.getItems(0)[1].value, 555);

    // moved-from object should reflect move (per MoveTrack behavior)
    EXPECT_EQ(m2.value, -1);
    EXPECT_TRUE(m2.movedFrom);
}

TEST(VersionedBucketSorterTest, EmplaceVariadicConstructor)
{
    VersionedBucketList<Emplaceable> sorter(3);

    sorter.emplaceItem(1, 11, 22, std::string("hello"));

    ASSERT_TRUE(sorter.hasItems(1));
    const auto& items = sorter.getItems(1);
    ASSERT_EQ(items.size(), 1u);
    EXPECT_EQ(items[0], Emplaceable(11, 22, "hello"));
}

TEST(VersionedBucketSorterTest, MultipleBucketsIndependenceAndBoundaries)
{
    const size_t buckets = 5;
    VersionedBucketList<std::string> sorter(buckets);

    sorter.emplaceItem(0, std::string("zero"));
    sorter.emplaceItem(buckets - 1, std::string("last"));
    sorter.emplaceItem(2, std::string("middle"));

    EXPECT_TRUE(sorter.hasItems(0));
    EXPECT_TRUE(sorter.hasItems(2));
    EXPECT_TRUE(sorter.hasItems(buckets - 1));

    EXPECT_EQ(sorter.getItems(0).size(), 1u);
    EXPECT_EQ(sorter.getItems(2).size(), 1u);
    EXPECT_EQ(sorter.getItems(buckets - 1).size(), 1u);

    EXPECT_EQ(sorter.getItems(0)[0], "zero");
    EXPECT_EQ(sorter.getItems(2)[0], "middle");
    EXPECT_EQ(sorter.getItems(buckets - 1)[0], "last");
}

TEST(VersionedBucketSorterTest, RepeatedEmplaceKeepsOrderAndValues)
{
    VersionedBucketList<int> sorter(1);

    for (int i = 0; i < 20; ++i)
    {
        sorter.emplaceItem(0, i);
    }

    const auto& items = sorter.getItems(0);
    ASSERT_EQ(items.size(), 20u);
    for (int i = 0; i < 20; ++i)
        EXPECT_EQ(items[static_cast<size_t>(i)], i);
}

TEST(VersionedBucketSorterTest, VerionIncrementInvalidatingItems)
{
    const size_t buckets = 5;
    VersionedBucketList<std::string> sorter(buckets);

    sorter.emplaceItem(2, std::string("something"));

    EXPECT_TRUE(sorter.hasItems(2));
    EXPECT_EQ(sorter.getItems(2).size(), 1u);
    EXPECT_EQ(sorter.getItems(2)[0], "something");

    sorter.incrementVersion();

    EXPECT_FALSE(sorter.hasItems(2));
}

TEST(VersionedBucketSorterTest, VerionIncrementAndAddingNewItems)
{
    const size_t buckets = 5;
    VersionedBucketList<std::string> sorter(buckets);

    sorter.emplaceItem(2, std::string("v1_item"));

    EXPECT_TRUE(sorter.hasItems(2));
    EXPECT_EQ(sorter.getItems(2).size(), 1u);
    EXPECT_EQ(sorter.getItems(2)[0], "v1_item");

    sorter.incrementVersion();

    sorter.emplaceItem(2, std::string("v2_item1"));
    sorter.emplaceItem(2, std::string("v2_item2"));

    EXPECT_TRUE(sorter.hasItems(2));
    EXPECT_EQ(sorter.getItems(2).size(), 2u);
    EXPECT_EQ(sorter.getItems(2)[0], "v2_item1");
    EXPECT_EQ(sorter.getItems(2)[1], "v2_item2");
}

TEST(VersionedBucketSorterTest, VerionIncrementAndGetItemsWithoutAdding)
{
    const size_t buckets = 5;
    VersionedBucketList<std::string> sorter(buckets);

    sorter.emplaceItem(2, std::string("v1_item"));

    EXPECT_TRUE(sorter.hasItems(2));
    EXPECT_EQ(sorter.getItems(2).size(), 1u);
    EXPECT_EQ(sorter.getItems(2)[0], "v1_item");

    sorter.incrementVersion();

    EXPECT_FALSE(sorter.hasItems(2));
    EXPECT_EQ(sorter.getItems(2).size(), 0);
}

} // namespace core