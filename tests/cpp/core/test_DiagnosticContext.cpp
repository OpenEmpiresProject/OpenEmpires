#include <gtest/gtest.h>

#include <string>
#include <thread>
#include <future>
#include <regex>

#include "logging/DiagnosticContext.h"

namespace core
{

class DiagnosticContextTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        DiagnosticContext::getInstance().clear();
    }
};

TEST_F(DiagnosticContextTest, EmptyInitiallyProducesEmptyString)
{
    auto& ctx = DiagnosticContext::getInstance();
    EXPECT_EQ(ctx.format(), "");
}

TEST_F(DiagnosticContextTest, PutPrimitiveAndStringFormatsSortedKeyValuePairs)
{
    auto& ctx = DiagnosticContext::getInstance();

    // Insert in non-sorted order
    ctx.put("zeta", 100);
    ctx.put("alpha", std::string("one"));

    // Expect alphabetical order: alpha then zeta
    EXPECT_EQ(ctx.format(), "alpha=one, zeta=100");
}

TEST_F(DiagnosticContextTest, PutSameValueDoesNotChangeFormattedOutput)
{
    auto& ctx = DiagnosticContext::getInstance();

    ctx.put("a", 1);
    ctx.put("b", 2);

    auto first = ctx.format();
    // Put identical value again - should not change representation
    ctx.put("a", 1);
    auto second = ctx.format();

    EXPECT_EQ(first, second);
    EXPECT_EQ(first, "a=1, b=2");
}

TEST_F(DiagnosticContextTest, UpdatingValueChangesFormattedOutput)
{
    auto& ctx = DiagnosticContext::getInstance();

    ctx.put("x", 1);
    EXPECT_EQ(ctx.format(), "x=1");

    ctx.put("x", 42);
    EXPECT_EQ(ctx.format(), "x=42");
}

TEST_F(DiagnosticContextTest, RemoveKeyExcludesItFromFormat)
{
    auto& ctx = DiagnosticContext::getInstance();

    ctx.put("k1", "one");
    ctx.put("k2", "two");
    EXPECT_EQ(ctx.format(), "k1=one, k2=two");

    ctx.remove("k1");
    EXPECT_EQ(ctx.format(), "k2=two");

    // Removing non-existing key should be safe / no throw
    ctx.remove("does-not-exist");
    EXPECT_EQ(ctx.format(), "k2=two");
}

TEST_F(DiagnosticContextTest, ClearRemovesAllEntries)
{
    auto& ctx = DiagnosticContext::getInstance();

    ctx.put("a", 1);
    ctx.put("b", 2);
    EXPECT_NE(ctx.format(), "");

    ctx.clear();
    EXPECT_EQ(ctx.format(), "");
}

struct Printable
{
    int v;
    friend std::ostream& operator<<(std::ostream& os, Printable const& p)
    {
        return os << "Printable(" << p.v << ")";
    }
};

TEST_F(DiagnosticContextTest, SupportsOstreamableTypes)
{
    auto& ctx = DiagnosticContext::getInstance();
    ctx.put("p", Printable{7});

    EXPECT_EQ(ctx.format(), "p=Printable(7)");
}

TEST_F(DiagnosticContextTest, UnformattableFallsBackToPlaceholder)
{
    struct NoStream
    {
        int a = 5;
    };

    auto& ctx = DiagnosticContext::getInstance();
    ctx.put("u", NoStream{});

    auto out = ctx.format();
    // Expect placeholder that starts with "<unformattable:"
    EXPECT_TRUE(out.rfind("u=<unformattable:", 0) == 0 ||
                std::regex_search(out, std::regex("u=<unformattable:.*>")));
}

TEST_F(DiagnosticContextTest, KeysAreSortedAfterMultiplePuts)
{
    auto& ctx = DiagnosticContext::getInstance();

    ctx.put("z", 1);
    ctx.put("m", 2);
    ctx.put("a", 3);
    // Because put sorts keys for newly inserted ones, the output must be ordered alphabetically
    EXPECT_EQ(ctx.format(), "a=3, m=2, z=1");
}

TEST_F(DiagnosticContextTest, ThreadLocalContextIsIndependent)
{
    auto& mainCtx = DiagnosticContext::getInstance();
    mainCtx.put("t", std::string("main"));

    // Start a new thread and set a different value in that thread's DiagnosticContext.
    auto fut = std::async(std::launch::async,
                          []() -> std::string
                          {
                              auto& ctx = DiagnosticContext::getInstance();
                              ctx.clear(); // ensure fresh for this thread
                              ctx.put("t", std::string("thread"));
                              return ctx.format();
                          });

    auto threadResult = fut.get();
    // Thread's context should reflect thread value
    EXPECT_EQ(threadResult, "t=thread");

    // Main thread's context should be unchanged
    EXPECT_EQ(mainCtx.format(), "t=main");
}

TEST_F(DiagnosticContextTest, PutExistingKeyDoesNotDuplicateSortList)
{
    auto& ctx = DiagnosticContext::getInstance();
    ctx.put("a", 1);
    ctx.put("a", 2); // update, should NOT duplicate "a" in m_keysSorted

    EXPECT_EQ(ctx.format(), "a=2");
}

TEST_F(DiagnosticContextTest, UpdatingExistingKeyDoesNotChangeOrdering)
{
    auto& ctx = DiagnosticContext::getInstance();
    ctx.put("b", 1);
    ctx.put("a", 1);
    ctx.put("b", 2);

    EXPECT_EQ(ctx.format(), "a=1, b=2");
}

TEST_F(DiagnosticContextTest, RemoveThenPutRestoresKeyInCorrectOrder)
{
    auto& ctx = DiagnosticContext::getInstance();

    ctx.put("x", 1);
    ctx.put("y", 2);
    ctx.remove("x");
    ctx.put("x", 3);

    EXPECT_EQ(ctx.format(), "x=3, y=2");
}

TEST_F(DiagnosticContextTest, SpacingAndDelimiterConsistency)
{
    auto& ctx = DiagnosticContext::getInstance();
    ctx.put("z", 9);
    ctx.put("a", 1);
    ctx.put("m", 5);
    ctx.remove("m");
    ctx.put("m", 7);

    EXPECT_EQ(ctx.format(), "a=1, m=7, z=9");
}

} // namespace core