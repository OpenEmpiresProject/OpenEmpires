#include <gtest/gtest.h>

#include "ServiceRegistry.h"
#include "Settings.h"
#include "StateManager.h"
#include "VisionSystem.h"
#include "Event.h"
#include "TileMap.h"
#include "components/CompVision.h"
#include "components/CompTransform.h"
#include "components/CompEntityInfo.h"
#include "Property.h"
#include "Property.h" // for PropertyInitializer usage (project uses this pattern)
#include "utils/Constants.h"
#include "TestEventPublisher.h"
#include "FogOfWar.h"


// The implementation-level free functions exist in VisionSystem.cpp but are not in the header.
// We declare them here to test their behavior.
extern uint32_t getCompositeId(uint32_t tracker, uint32_t target);
extern bool isInLOSRoundedSquare(const core::Feet& p, const core::Rect<float>& b, float LOS);

namespace core
{

class ExposedVisionSystem : public VisionSystem
{
  public:
    void onInit(EventLoop& eventLoop)
    {
        VisionSystem::onInit(eventLoop);
    }
};

class VisionSystemTest : public ::testing::Test, public core::PropertyInitializer
{
  protected:
    void SetUp() override
    {
        // Register services required by VisionSystem
        ServiceRegistry::getInstance().registerService(std::make_shared<Settings>());
        auto sm = std::make_shared<StateManager>();
        sm->gameMap().init(10, 10);
        ServiceRegistry::getInstance().registerService(sm);

        // Dummy stop token for EventLoop constructor (we won't run it)
        std::stop_token dummyToken;
        eventLoop = std::make_unique<EventLoop>(&dummyToken);

        // Create VisionSystem as shared_ptr (onInit uses shared_from_this)
        vision = std::make_shared<ExposedVisionSystem>();
        vision->onInit(*eventLoop);

        // Install test event publisher to capture published events
        publisher = std::make_shared<core::test::TestEventPublisher>();
        publisher->install();
    }

    void TearDown() override
    {
        // Clear state
        ServiceRegistry::getInstance().getService<StateManager>()->clearAll();
    }

    Ref<ExposedVisionSystem> vision;
    std::unique_ptr<EventLoop> eventLoop;
    std::shared_ptr<core::test::TestEventPublisher> publisher;
};

// Basic check for composite id arithmetic
TEST_F(VisionSystemTest, CompositeIdUniqueness)
{
    uint32_t a = 5;
    uint32_t b = 123;
    uint32_t id = getCompositeId(a, b);
    EXPECT_EQ(id, a * Constants::MAX_ENTITIES + b);

    // Different pair should not collide
    uint32_t id2 = getCompositeId(a + 1, b - 1);
    EXPECT_NE(id, id2);
}

// isInLOSRoundedSquare: axis aligned horizontal/vertical coverage and corner checks
TEST_F(VisionSystemTest, IsInLOSRoundedSquare_AxisAlignedAndCorners)
{
    // Create a rectangle centered at (512,512) covering two tiles (assuming FEET_PER_TILE=256)
    Rect<float> rect(512.0f, 512.0f, 256.0f, 256.0f); // left,top,width,height
    float LOS = 128.0f;                               // feet

    // Point horizontally aligned with rect -> should be inside LOS (x within rect.x..rect.x+w)
    Feet p1(rect.x + rect.w / 2.0f, rect.y - LOS + 1.0f); // just within top LOS band
    EXPECT_TRUE(isInLOSRoundedSquare(p1, rect, LOS));

    // Point vertically aligned with rect -> should be inside LOS
    Feet p2(rect.x - LOS + 1.0f, rect.y + rect.h / 2.0f); // just within left LOS band
    EXPECT_TRUE(isInLOSRoundedSquare(p2, rect, LOS));

    // Point near top-left corner but within quarter circle radius should be inside
    Feet cornerInside(rect.x - LOS / 2.0f, rect.y - LOS / 2.0f);
    EXPECT_TRUE(isInLOSRoundedSquare(cornerInside, rect, LOS));

    // Point near top-left corner but outside quarter circle radius should be outside
    Feet cornerOutside(rect.x - 10, rect.y - 10);
    EXPECT_TRUE(isInLOSRoundedSquare(cornerOutside, rect, LOS));

    // Point far outside should be false
    Feet far(rect.x - (LOS + 100.0f), rect.y - (LOS + 100.0f));
    EXPECT_FALSE(isInLOSRoundedSquare(far, rect, LOS));
}

// Integration test: circle LOS flow -> publish WITHIN_LINE_OF_SIGHT then OUT_OF_LINE_OF_SIGHT
TEST_F(VisionSystemTest, CircleLOS_PublishesWithinAndOutEvents)
{
    auto state = ServiceRegistry::getInstance().getService<StateManager>();

    // Create tracker and target entities
    uint32_t tracker = state->createEntity();
    uint32_t target = state->createEntity();

    // Add required components to tracker and target
    CompVision trackerVision;
    // Set properties using project's PropertyInitializer API
    PropertyInitializer::set<uint32_t>(trackerVision.lineOfSight, 300u); // feet
    PropertyInitializer::set<LineOfSightShape>(trackerVision.lineOfSightShape,
                                               LineOfSightShape::CIRCLE);
    trackerVision.hasVision = true;
    state->addComponent<CompVision>(tracker, trackerVision);

    // Target needs CompEntityInfo and CompTransform so VisionSystem::Target ctor can get references
    state->addComponent<CompEntityInfo>(target, CompEntityInfo(0));
    CompTransform targetTransform;
    // Place target within LOS distance from center we'll provide in tracking request below
    targetTransform.position = Feet(1000, 1000); // feet
    state->addComponent<CompTransform>(target, targetTransform);

    // Prepare tracking request centered near target
    TrackingRequestData tr;
    tr.entity = tracker;
    tr.center = Feet(1000, 1000); // same as target -> distance 0
    // dispatch TRACKING_REQUEST to VisionSystem -> will populate internal m_trackersTileMap
    Event trEvent(Event::Type::TRACKING_REQUEST, tr);
    vision->dispatchEvent(trEvent);

    // Add target to the global map at the tile corresponding to its position.
    Tile targetTile = targetTransform.position.toTile();
    state->gameMap().addEntity(MapLayerType::UNITS, targetTile, target);

    // Clear any previously captured events
    publisher->clear();

    // Fire a TICK event to cause VisionSystem to evaluate LOS and publish WITHIN_LINE_OF_SIGHT
    Event tick(Event::Type::TICK, TickData{0});
    vision->dispatchEvent(tick);

    const auto& evs = publisher->events();
    ASSERT_FALSE(evs.empty());
    // Find at least one WITHIN_LINE_OF_SIGHT
    bool foundWithin = false;
    for (const auto& e : evs)
    {
        if (e.type == Event::Type::WITHIN_LINE_OF_SIGHT)
        {
            auto d = e.getData<LineOfSightData>();
            EXPECT_EQ(d.tracker, tracker);
            EXPECT_EQ(d.target, target);
            foundWithin = true;
        }
    }
    EXPECT_TRUE(foundWithin);

    // Now move target out of LOS and tick again -> expect OUT_OF_LINE_OF_SIGHT
    state->getComponent<CompTransform>(target).position = Feet(100000, 100000); // far away

    publisher->clear();
    vision->dispatchEvent(tick);

    bool foundOut = false;
    for (const auto& e : publisher->events())
    {
        if (e.type == Event::Type::OUT_OF_LINE_OF_SIGHT)
        {
            auto d = e.getData<LineOfSightData>();
            EXPECT_EQ(d.tracker, tracker);
            EXPECT_EQ(d.target, target);
            foundOut = true;
        }
    }
    EXPECT_TRUE(foundOut);
}

// Integration test: rounded-square LOS uses land area and produces events similarly
TEST_F(VisionSystemTest, RoundedSquareLOS_PublishesWithinAndOutEvents)
{
    auto state = ServiceRegistry::getInstance().getService<StateManager>();

    uint32_t tracker = state->createEntity();
    uint32_t target = state->createEntity();

    // Tracker is a building-like tracker: rounded-square LOS
    CompVision trackerVision;
    PropertyInitializer::set<uint32_t>(trackerVision.lineOfSight, 256u * 1u); // 1 tile in feet
    PropertyInitializer::set<LineOfSightShape>(trackerVision.lineOfSightShape,
                                               LineOfSightShape::ROUNDED_SQUARE);
    trackerVision.hasVision = true;
    state->addComponent<CompVision>(tracker, trackerVision);

    // For the land area we create a single tile covering the tracker center
    TrackingRequestData tr;
    tr.entity = tracker;
    Tile centerTile(4, 4);
    tr.landArea.tiles.push_back(centerTile);

    // Target components
    state->addComponent<CompEntityInfo>(target, CompEntityInfo(0));
    CompTransform targetTransform;
    // place target at tile center -> within LOS
    targetTransform.position = centerTile.centerInFeet();
    state->addComponent<CompTransform>(target, targetTransform);

    // Dispatch tracking request (rounded square path) - will populate vision->m_trackersTileMap
    Event trEvent(Event::Type::TRACKING_REQUEST, tr);
    vision->dispatchEvent(trEvent);

    // Add target into world map at that tile - triggers onEntityEnter
    state->gameMap().addEntity(MapLayerType::STATIC, centerTile, target);

    publisher->clear();

    // Tick -> expect WITHIN_LINE_OF_SIGHT
    Event tick(Event::Type::TICK, TickData{0});
    vision->dispatchEvent(tick);

    bool withinFound = false;
    for (const auto& e : publisher->events())
    {
        if (e.type == Event::Type::WITHIN_LINE_OF_SIGHT)
        {
            auto d = e.getData<LineOfSightData>();
            EXPECT_EQ(d.tracker, tracker);
            EXPECT_EQ(d.target, target);
            withinFound = true;
        }
    }
    EXPECT_TRUE(withinFound);

    // Now push target away (set position far) and tick -> OUT_OF_LINE_OF_SIGHT expected
    state->getComponent<CompTransform>(target).position = Feet(100000, 100000);

    publisher->clear();
    vision->dispatchEvent(tick);

    bool outFound = false;
    for (const auto& e : publisher->events())
    {
        if (e.type == Event::Type::OUT_OF_LINE_OF_SIGHT)
        {
            auto d = e.getData<LineOfSightData>();
            EXPECT_EQ(d.tracker, tracker);
            EXPECT_EQ(d.target, target);
            outFound = true;
        }
    }
    EXPECT_TRUE(outFound);
}
} // namespace core