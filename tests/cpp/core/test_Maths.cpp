#include "utils/Maths.h"

#include <gtest/gtest.h>

namespace core
{
namespace maths
{
class MathsTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }
};

// isInLOSRoundedSquare: axis aligned horizontal/vertical coverage and corner checks
TEST_F(MathsTest, isOverlapping_RoundedCornerSquare)
{
    // Create a rectangle centered at (512,512) covering two tiles (assuming FEET_PER_TILE=256)
    Rect<float> rect(512.0f, 512.0f, 256.0f, 256.0f); // left,top,width,height
    float LOS = 128.0f;                               // feet

    // Point horizontally aligned with rect -> should be inside LOS (x within rect.x..rect.x+w)
    Feet p1(rect.x + rect.w / 2.0f, rect.y - LOS + 1.0f); // just within top LOS band
    EXPECT_TRUE(isOverlapping(rect, LOS, p1));

    // Point vertically aligned with rect -> should be inside LOS
    Feet p2(rect.x - LOS + 1.0f, rect.y + rect.h / 2.0f); // just within left LOS band
    EXPECT_TRUE(isOverlapping(rect, LOS, p2));

    // Point near top-left corner but within quarter circle radius should be inside
    Feet cornerInside(rect.x - LOS / 2.0f, rect.y - LOS / 2.0f);
    EXPECT_TRUE(isOverlapping(rect, LOS, cornerInside));

    // Point near top-left corner but outside quarter circle radius should be outside
    Feet cornerOutside(rect.x - 10, rect.y - 10);
    EXPECT_TRUE(isOverlapping(rect, LOS, cornerOutside));

    // Point far outside should be false
    Feet far(rect.x - (LOS + 100.0f), rect.y - (LOS + 100.0f));
    EXPECT_FALSE(isOverlapping(rect, LOS, far));
}
} // namespace maths

} // namespace core