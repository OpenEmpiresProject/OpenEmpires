#include "LineUnitFormation.h"
#include "ServiceRegistry.h"
#include "Settings.h"
#include "StateManager.h"
#include "components/CompTransform.h"

#include <gtest/gtest.h>

namespace core
{
static Feet anchorFeet(5000, 5000);

static uint32_t createUnit(StateManager& stateMan, const Feet& pos, int collisionRadius = 100)
{
    uint32_t e = stateMan.createEntity();
    CompTransform t(pos);
    t.collisionRadius = collisionRadius;
    stateMan.addComponent<CompTransform>(e, t);
    return e;
}

class LineUnitFormationTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        // Register Settings (StateManager init depends on settings)
        m_settings = CreateRef<Settings>();
        m_settings->setWorldSizeType(WorldSizeType::TEST);
        ServiceRegistry::getInstance().registerService<Settings>(m_settings);

        m_stateMan = CreateRef<StateManager>();
        m_stateMan->init();
        ServiceRegistry::getInstance().registerService<StateManager>(m_stateMan);
    }

    void TearDown() override
    {
        // clear registry state to avoid test interference
        m_stateMan->clearAll();
    }

    Ref<Settings> m_settings;
    Ref<StateManager> m_stateMan;
};

TEST_F(LineUnitFormationTest, OddNumberOfUnits_AssignsAllUnits)
{
    // 5 (odd) units
    const size_t count = 5;
    std::vector<uint32_t> units;
    units.reserve(count);

    // place units near anchor with small offsets
    units.push_back(createUnit(*m_stateMan, anchorFeet + Feet(0, 0), 100));
    units.push_back(createUnit(*m_stateMan, anchorFeet + Feet(150, 0), 100));
    units.push_back(createUnit(*m_stateMan, anchorFeet + Feet(-150, 0), 100));
    units.push_back(createUnit(*m_stateMan, anchorFeet + Feet(0, 150), 100));
    units.push_back(createUnit(*m_stateMan, anchorFeet + Feet(0, -150), 100));

    auto formation = std::make_shared<LineUnitFormation>();
    formation->createFormation(units, anchorFeet);

    const auto& slots = formation->getSlots();
    EXPECT_EQ(slots.size(), count); // all units assigned
}

TEST_F(LineUnitFormationTest, ClosestUnitAllocatedToSlot)
{
    // 2 units placed so one is clearly closer to slot 0 and the other to slot 1
    std::vector<uint32_t> units;
    units.reserve(2);

    // We'll choose collision radius such that spacing predictable
    int collR = 100;
    uint32_t a = createUnit(*m_stateMan, anchorFeet + Feet(-10, -10),
                            collR); // should be nearest to first slot (center-ish)
    uint32_t b = createUnit(*m_stateMan, anchorFeet + Feet(500, 0),
                            collR); // should be nearest to a right slot

    units.push_back(a);
    units.push_back(b);

    auto formation = std::make_shared<LineUnitFormation>();
    formation->createFormation(units, anchorFeet);

    const auto& slots = formation->getSlots();
    ASSERT_EQ(slots.size(), 2u);

    // Recompute nearest manually for each assigned slot using actual formation anchor
    for (const auto& s : slots)
    {
        Feet worldSlotPos = formation->getAnchor() + s.offsetFromAnchor;
        // compute distances
        float distA =
            (worldSlotPos - m_stateMan->getComponent<CompTransform>(a).position).lengthSquared();
        float distB =
            (worldSlotPos - m_stateMan->getComponent<CompTransform>(b).position).lengthSquared();

        if (distA < distB)
            EXPECT_EQ(s.getEntityId(), a);
        else
            EXPECT_EQ(s.getEntityId(), b);
    }
}
TEST_F(LineUnitFormationTest, PackedSizeBasedOnUsedSlots)
{
    // Create 5 units; the grid may be 3x2; verify actual used bounding box based on assigned slots.
    const size_t count = 5;
    std::vector<uint32_t> units;
    units.reserve(count);

    units.push_back(createUnit(*m_stateMan, anchorFeet + Feet(0, 0), 100));
    units.push_back(createUnit(*m_stateMan, anchorFeet + Feet(200, 0), 100));
    units.push_back(createUnit(*m_stateMan, anchorFeet + Feet(-200, 0), 100));
    units.push_back(createUnit(*m_stateMan, anchorFeet + Feet(0, 200), 100));
    units.push_back(createUnit(*m_stateMan, anchorFeet + Feet(0, -200), 100));

    // For this configuration maxRadius == 100 => cellSpacing = 120, startX = -120, startY = -60
    const float cellSpacing = 120.0f;

    auto formation = std::make_shared<LineUnitFormation>();
    formation->createFormation(units, anchorFeet);

    const auto& slots = formation->getSlots();
    ASSERT_EQ(slots.size(), count);

    // Determine used slot x indices (robust to slot index ordering).
    float minX = std::numeric_limits<float>::max(), maxX = std::numeric_limits<float>::lowest();
    std::set<int> uniqueCols;

    for (const auto& s : slots)
    {
        // normalize by cellSpacing and round to nearest integer to find column index
        int colIdx = static_cast<int>(std::lround(s.offsetFromAnchor.x / cellSpacing));
        uniqueCols.insert(colIdx);

        minX = std::min(minX, s.offsetFromAnchor.x);
        maxX = std::max(maxX, s.offsetFromAnchor.x);
    }

    int usedColumns = static_cast<int>(uniqueCols.size());
    float expectedPackedWidth = usedColumns * cellSpacing;
    float actualPackedWidth = (maxX - minX) + cellSpacing; // extents plus one cell width

    EXPECT_NEAR(actualPackedWidth, expectedPackedWidth, 1e-3f);
}

TEST_F(LineUnitFormationTest, DifferentCollisionRadii_AccommodatesLargestRadius)
{
    // Units with different collision radii. Ensure spacing is computed using largest radius.
    std::vector<uint32_t> units;
    units.reserve(3);

    // radii: 50, 150, 100 -> max is 150
    units.push_back(createUnit(*m_stateMan, anchorFeet + Feet(-200, 0), 50));
    units.push_back(createUnit(*m_stateMan, anchorFeet + Feet(0, 0), 150));
    units.push_back(createUnit(*m_stateMan, anchorFeet + Feet(200, 0), 100));

    // expected spacing for maxRadius == 150
    const float expectedCellSpacing = 150.0f * 2 * LineUnitFormation().SPACING_FACTOR;

    auto formation = std::make_shared<LineUnitFormation>();
    formation->createFormation(units, anchorFeet);

    const auto& slots = formation->getSlots();
    ASSERT_GE(slots.size(), 2u);

    // Simplified verification: take the first two slots and check their x-gap matches expected
    // spacing.
    float dx = std::abs(slots[0].offsetFromAnchor.y - slots[1].offsetFromAnchor.y);
    EXPECT_NEAR(dx, expectedCellSpacing, 1e-3f);
}

// New test: validate anchor computed as center of mass of unit positions
TEST_F(LineUnitFormationTest, Anchor_CalculatedAsCenterOfMass_ReturnsAveragePosition)
{
    std::vector<uint32_t> units;
    units.reserve(3);

    // Place units at known absolute positions
    uint32_t u1 = createUnit(*m_stateMan, Feet(1000.0f, 1000.0f), 100);
    uint32_t u2 = createUnit(*m_stateMan, Feet(2000.0f, 1000.0f), 100);
    uint32_t u3 = createUnit(*m_stateMan, Feet(1000.0f, 3000.0f), 100);

    units.push_back(u1);
    units.push_back(u2);
    units.push_back(u3);

    auto formation = std::make_shared<LineUnitFormation>();
    formation->createFormation(units, anchorFeet);

    // Compute expected center of mass (average of actual unit world positions)
    Feet sum = Feet::zero;
    for (auto id : units)
    {
        sum += m_stateMan->getComponent<CompTransform>(id).position;
    }
    Feet expected = sum / static_cast<float>(units.size());

    Feet anchor = formation->getAnchor();
    // Anchor must be set and approximately equal to expected center of mass
    EXPECT_FALSE(anchor.isNull());
    EXPECT_NEAR(anchor.x, expected.x, 1e-3f);
    EXPECT_NEAR(anchor.y, expected.y, 1e-3f);
}
} // namespace core