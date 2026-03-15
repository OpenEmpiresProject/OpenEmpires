#include "PassabilityMap.h"
#include "Path.h"
#include "PathService.h"
#include "Player.h"
#include "PlayerFactory.h"
#include "ServiceRegistry.h"
#include "Settings.h"
#include "StateManager.h"
#include "utils/Types.h"

#include <gtest/gtest.h>
#include "components/CompTransform.h"

namespace core
{

class PathServiceTest : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        // Register small test world settings first (Player::init depends on Settings).
        m_settings = CreateRef<Settings>();
        m_settings->setWorldSizeType(WorldSizeType::TEST); // 10x10 tiles
        ServiceRegistry::getInstance().registerService<Settings>(m_settings);

        // Create and register a StateManager and initialize maps.
        m_stateMan = CreateRef<StateManager>();
        m_stateMan->init();
        ServiceRegistry::getInstance().registerService<StateManager>(m_stateMan);

        // Create a player with id 1.
        m_player = CreateRef<Player>();
        m_player->init(1);

        // PathService under test.
        m_pathService = std::make_unique<PathService>();
    }

    void TearDown() override
    {
        // Clear any state manager contents between tests to avoid cross-test interference.
        m_stateMan->clearAll();
    }

    Ref<Settings> m_settings;
    Ref<StateManager> m_stateMan;
    Ref<Player> m_player;
    std::unique_ptr<PathService> m_pathService;
};

// Helper to produce Feet by tile coordinates (center of tile).
static Feet tileCenterFeet(int tileX, int tileY)
{
    // Use Coordinates::getTileCenterInFeet semantics: tile * FEET_PER_TILE + half
    const int half = Constants::FEET_PER_TILE / 2;
    return Feet(tileX * Constants::FEET_PER_TILE + half, tileY * Constants::FEET_PER_TILE + half);
}

// 1) canTraverseDirectly: when all tiles along the sampled line are passable -> true
/*
 *   . . . . .
 *   . S . E .
 *   . . . . .
 *   . . . . .
 *   . . . . .
 */
TEST_F(PathServiceTest, CanTraverseDirectly_AllPassable_ReturnsTrue)
{
    auto from = tileCenterFeet(1, 1);
    auto to = tileCenterFeet(4, 1); // horizontal line across tiles (1..4)

    // Ensure passability map default is passable (PASSABLE_FOR_ANY) after StateManager::init()
    EXPECT_TRUE(m_stateMan->getPassabilityMap().isPassableFor(from.toTile(), m_player->getId()));
    EXPECT_TRUE(m_stateMan->getPassabilityMap().isPassableFor(to.toTile(), m_player->getId()));

    bool traversable = m_pathService->canTraverseDirectly(from, to, m_player);
    EXPECT_TRUE(traversable);
}

// 2) canTraverseDirectly: when an intermediate tile is blocked -> false
/*
 *   S X . E .
 *   . . . . .
 *   . . . . .
 *   . . . . .
 *   . . . . .
 */
TEST_F(PathServiceTest, CanTraverseDirectly_BlockedTile_ReturnsFalse)
{
    auto from = tileCenterFeet(0, 0);
    auto to = tileCenterFeet(3, 0); // line across tiles x=0..3

    Tile blockedTile(1, 0);
    m_stateMan->getPassabilityMap().setTileDynamicPassability(blockedTile,
                                                              DynamicPassability::BLOCKED_FOR_ANY);

    // Sanity: blocked tile is not passable for this player
    EXPECT_FALSE(m_stateMan->getPassabilityMap().isPassableFor(blockedTile, m_player->getId()));

    bool traversable = m_pathService->canTraverseDirectly(from, to, m_player);
    EXPECT_FALSE(traversable);
}

// 3) refinePath: remove intermediate waypoints that are directly visible from the last kept
// waypoint
/*
 *   . . . . .
 *   . . . . .
 *   . . S _ E
 *   . . . . .
 *   . . . . .
 */
TEST_F(PathServiceTest, RefinePath_RemovesVisibleIntermediateWaypoints)
{
    // Create a simple path with origin -> mid -> dest where direct line origin->dest is clear.
    Feet origin = tileCenterFeet(2, 2);
    Feet mid = tileCenterFeet(3, 2);
    Feet dest = tileCenterFeet(4, 2);

    // Ensure all tiles used are passable
    EXPECT_TRUE(m_stateMan->getPassabilityMap().isPassableFor(origin.toTile(), m_player->getId()));
    EXPECT_TRUE(m_stateMan->getPassabilityMap().isPassableFor(mid.toTile(), m_player->getId()));
    EXPECT_TRUE(m_stateMan->getPassabilityMap().isPassableFor(dest.toTile(), m_player->getId()));

    std::vector<Feet> pts{origin, mid, dest};
    Path path(pts);

    // Confirm initial size is 3 (origin, mid, dest)
    EXPECT_EQ(path.getWaypoints().size(), 3u);

    // Call refinePath which should remove the middle waypoint because origin->mid is visible,
    // and origin can also see dest (so mid should be removed).
    m_pathService->refinePath(path, m_player);

    // After refinement, only origin and dest should remain.
    const auto& waypoints = path.getWaypoints();
    ASSERT_EQ(waypoints.size(), 2u);

    auto it = waypoints.begin();
    EXPECT_EQ(*it, origin);
    ++it;
    EXPECT_EQ(*it, dest);
}

// 4) findPath: when direct traversal is possible and distance is within threshold, pathfinding is
// skipped
/*
 *   . . . . . . . .
 *   . . . . . . . .
 *   . . . . . . . .
 *   . . S . . . . E
 *   . . . . . . . .
 */
TEST_F(PathServiceTest, FindPath_DirectTraversal_SkipsPathfinding)
{
    // Pick two tiles that are fairly close (within MAX_DIRECT_PATH_DISTANCE_IN_TILES)
    Feet from = tileCenterFeet(2, 3);
    Feet to = tileCenterFeet(7, 3); // 5 tiles apart, less than 10

    // All tiles are passable by default
    Path result = m_pathService->findPath(from, to, m_player);

    // Direct path returns Path({to}) (does not include origin).
    const auto& wps = result.getWaypoints();
    ASSERT_EQ(wps.size(), 1u);
    EXPECT_EQ(wps.front(), to);
}

// 5) findPath: when direct traversal is blocked, pathfinder is used and result must include origin
// and destination
/*
 *   . . . . . . . .
 *   . . . . . . . .
 *   . . . . . . . .
 *   . . . . . . . .
 *   . . . . . . . .
 *   . S . X . E . .
 */
TEST_F(PathServiceTest, FindPath_BlockedDirect_UsesPathfinder_IncludesEndpoints)
{
    Feet from = tileCenterFeet(1, 5);
    Feet to = tileCenterFeet(5, 5); // 4 tiles apart, but we will block direct line

    // Block a tile between them to force pathfinder usage
    Tile blocked(3, 5);
    m_stateMan->getPassabilityMap().setTileDynamicPassability(blocked,
                                                              DynamicPassability::BLOCKED_FOR_ANY);
    EXPECT_FALSE(m_stateMan->getPassabilityMap().isPassableFor(blocked, m_player->getId()));

    Path result = m_pathService->findPath(from, to, m_player);

    // When pathfinder is used, PathService will insert origin at the front and push destination at
    // the back.
    const auto& wps = result.getWaypoints();
    ASSERT_GE(wps.size(), 2u);
    EXPECT_EQ(wps.front(), from);
    // last element should be the destination
    auto it = wps.end();
    --it;
    EXPECT_EQ(*it, to);
}

// 6) refinePath: multiple-segment removals (non-consecutive) producing multiple straight lines.
//    Setup a polyline with two straight segments:
//      origin -> (1,0) -> (2,0) -> corner(4,0) -> (4,1) -> (4,2) -> dest(4,3)
//    Block tile (3,0) so origin can see up to (2,0) but not the corner (4,0).
//    Expected refinement: origin, corner(4,0), dest(4,3)
/*
 *  Original waypoints 
 *   S ──┐ X . .
 *   . X │ X . .
 *   . X │ X  . .
 *   . X │ X E .
 *   . . └──── .
 *   . . . . . . 
 * 
 *  Refined waypoints (marked o)
 *   o . o X . .
 *   . X . X . .
 *   . X . X . .
 *   . X . X o .
 *   . . o . o .
 *   . . . . . . 
 */
TEST_F(PathServiceTest, RefinePath_MultipleSegmentRemovals_KeepsCorners)
{
    Feet origin = tileCenterFeet(0, 0);
    Feet p10 = tileCenterFeet(1, 0);
    Feet p20 = tileCenterFeet(2, 0);
    Feet p22 = tileCenterFeet(2, 2);
    Feet p23 = tileCenterFeet(2, 3);
    Feet p24 = tileCenterFeet(2, 4);
    Feet p34 = tileCenterFeet(3, 4);
    Feet p44 = tileCenterFeet(4, 4);
    Feet dest = tileCenterFeet(4, 3);

    const std::vector<Tile> blockers = {
        Tile(3, 0), Tile(3, 1), Tile(3, 2), Tile(3, 3),
        Tile(1, 1), Tile(1, 2), Tile(1, 3)
    };

    for (const auto& b : blockers)
    {
        m_stateMan->getPassabilityMap().setTileDynamicPassability(
            b, DynamicPassability::BLOCKED_FOR_ANY);
        EXPECT_FALSE(m_stateMan->getPassabilityMap().isPassableFor(b, m_player->getId()));
    }

    std::vector<Feet> pts{origin, p10, p20, p22, p23, p24, p34, p44, dest};
    Path path(pts);

    // Refine path
    m_pathService->refinePath(path, m_player);

    // After refinement we expect origin, corner, dest to remain (intermediates on each straight
    // segment removed).
    const auto& wps = path.getWaypoints();
    ASSERT_EQ(wps.size(), 5u);

    auto it = wps.begin();
    EXPECT_EQ(*it, origin);
    ++it;
    EXPECT_EQ(*it, p20);
    ++it;
    EXPECT_EQ(*it, p24);
    ++it;
    EXPECT_EQ(*it, p44);
    ++it;
    EXPECT_EQ(*it, dest);
}


// Tests for getBestAvoidanceVector

TEST_F(PathServiceTest, GetBestAvoidanceVector_Forward_NoDensity_PicksForward)
{
    // Arrange
    const uint32_t speed = Constants::FEET_PER_TILE; // 256
    Feet currentPos = tileCenterFeet(2, 2);
    Feet preferredVector = Feet(1.0f, 0.0f); // move east
    float deltaTimeS = 1.0f;

    // Ensure density grid is clear
    m_stateMan->getDensityGrid().clear();

    // Act
    Feet chosen = m_pathService->getBestAvoidanceDirectionVector(currentPos, preferredVector, 100, entt::null);

    // Expectation: chosen equals preferredVector (east)
    EXPECT_EQ(chosen, preferredVector);
}

TEST_F(PathServiceTest, GetBestAvoidanceVector_ForwardBlocked_45Degree_Wins)
{
    // Arrange
    const uint32_t speed = Constants::FEET_PER_TILE; // 256
    Feet currentPos = tileCenterFeet(2, 2);
    Feet preferredVector = Feet(static_cast<float>(speed), 0.0f); // move east
    float deltaTimeS = 1.0f;

    // Clear grid then add density directly in front so forward candidate is penalized.
    m_stateMan->getDensityGrid().clear();
    // predicted pos for forward: current + (speed,0) * deltaTimeS => tile (3,2)
    Feet forwardPred = currentPos + Feet(1.0f, 0.0f) * 100 * 2;
    m_stateMan->getDensityGrid().incrementDensity(forwardPred); // single increment is sufficient

    // Act
    Feet chosen = m_pathService->getBestAvoidanceDirectionVector(currentPos, preferredVector, 100,
                                                                 entt::null);

    // Expectation: chosen should be rotated 45 degrees (one of the 45-degree candidates).
    Feet expected45 = preferredVector.normalized().rotated(45.0f);
    Feet expected315 = preferredVector.normalized().rotated(315.0f);

    // Either +45 or -45 (315) is acceptable depending on candidate ordering; check one of them.
    const bool is45 = (chosen == expected45);
    const bool is315 = (chosen == expected315);
    EXPECT_TRUE(is45 || is315) << "Chosen vector: " << chosen.toString() << " expected either "
                               << expected45.toString() << " or " << expected315.toString();
}

TEST_F(PathServiceTest, GetBestAvoidanceVector_ForwardAnd45Blocked_90Degree_Wins)
{
    // Arrange
    const uint32_t speed = Constants::FEET_PER_TILE; // 256
    Feet currentPos = tileCenterFeet(2, 2);
    Feet preferredVector = Feet(1.0f, 0.0f); // move east
    float deltaTimeS = 1.0f;

    m_stateMan->getDensityGrid().clear();

    // Block forward
    Feet forwardPred = currentPos + Feet(1.0f, 0.0f) * 100 * 2;
    m_stateMan->getDensityGrid().incrementDensity(forwardPred);

    // Block both 45-degree candidates strongly (two increments each to increase penalty)
    Feet pred45 =
        currentPos +
        (preferredVector.normalized().rotated(45.0f) * 100 * 2);
    Feet pred315 =
        currentPos +
        (preferredVector.normalized().rotated(315.0f) * 100 * 2);
    m_stateMan->getDensityGrid().incrementDensity(pred45);
    m_stateMan->getDensityGrid().incrementDensity(pred45);
    m_stateMan->getDensityGrid().incrementDensity(pred315);
    m_stateMan->getDensityGrid().incrementDensity(pred315);

    // Act
    Feet chosen = m_pathService->getBestAvoidanceDirectionVector(currentPos, preferredVector, 100,
                                                                 entt::null);

    // Expectation: 90-degree rotated candidate (i.e. north or south depending on rotation sign).
    Feet expected90 = preferredVector.normalized().rotated(90.0f);
    Feet expected270 = preferredVector.normalized().rotated(270.0f);

    const bool is90 = (chosen == expected90);
    const bool is270 = (chosen == expected270);
    EXPECT_TRUE(is90 || is270) << "Chosen vector: " << chosen.toString() << " expected either "
                               << expected90.toString() << " or " << expected270.toString();
}

TEST_F(PathServiceTest, GetBestAvoidanceVector_BoxedExceptBehind_GoesBackward)
{
    // Arrange
    const uint32_t speed = Constants::FEET_PER_TILE; // 256
    Feet currentPos = tileCenterFeet(5, 5);
    Feet preferredVector = Feet(static_cast<float>(speed), 0.0f); // move east
    float deltaTimeS = 1.0f;

    // Ensure density grid is clear
    m_stateMan->getDensityGrid().clear();

    // Prepare candidate directions and heavily penalize all except the behind direction.
    auto preferredDir = preferredVector.normalized();
    auto candidates = m_pathService->generateCandidateDirections(preferredDir, 8);

    Feet behindDir = preferredDir.rotated(180.0f).normalized();

    auto isSameDir = [&](const Feet& a, const Feet& b) {
        auto an = a.normalized();
        auto bn = b.normalized();
        return std::abs(an.x - bn.x) < 1e-2f && std::abs(an.y - bn.y) < 1e-2f;
    };

    // Penalize every candidate except the behind one by incrementing density at predicted pos.
    for (const auto& dir : candidates)
    {
        if (isSameDir(dir, behindDir))
            continue;

        Feet predPos = currentPos + dir * 100 * 2;
        // Add several increments to make the density penalty dominant.
        m_stateMan->getDensityGrid().incrementDensity(predPos);
        m_stateMan->getDensityGrid().incrementDensity(predPos);
        m_stateMan->getDensityGrid().incrementDensity(predPos);
    }

    // Act
    Feet chosen = m_pathService->getBestAvoidanceDirectionVector(currentPos, preferredVector, 100,
                                                                 entt::null);

    // Expectation: chosen should be the backwards vector (180 degrees).
    Feet expectedBack = behindDir;
    EXPECT_EQ(chosen, expectedBack) << "Chosen vector: " << chosen.toString()
                                    << " expected back vector: " << expectedBack.toString();
}


// Tests for getSeparationPenalty

// Helper to create a unit with a transform and place it on the units layer.
static uint32_t createUnitAt(StateManager& stateMan, const Feet& pos, int collisionRadius = 100)
{
    uint32_t e = stateMan.createEntity();
    CompTransform t(pos);
    t.collisionRadius = collisionRadius;
    stateMan.addComponent<CompTransform>(e, t);
    stateMan.gameMap().addEntity(MapLayerType::UNITS, pos.toTile(), e);
    return e;
}

// 1) Search area crosses map boundary — ensure invalid tiles are skipped and penalty still
// computed.
TEST_F(PathServiceTest, GetSeparationPenalty_SearchCrossesBoundary_IgnoresInvalidTiles)
{
    // Position near (0,0) border
    Feet pos = tileCenterFeet(0, 0);

    // Place another unit slightly to the right such that its tile is (1,0) (valid),
    // and distance produces a known overlap.
    Feet otherPos = pos + Feet(150.0f, 0.0f); // dist = 150
    uint32_t other = createUnitAt(*m_stateMan, otherPos, 100);

    // unitCollisionRadius passed as 100, other collision radius default 100 => separationRadius =
    // 200
    float penalty = m_pathService->getSeparationPenalty(pos, entt::null, 100);

    // expected overlap = 200 - 150 = 50
    EXPECT_NEAR(penalty, 50.0f, 1e-3f);
}

// 2) Ignore unit itself: entity that equals unitEntity must be skipped.
TEST_F(PathServiceTest, GetSeparationPenalty_IgnoresSelf_ReturnsZero)
{
    Feet pos = tileCenterFeet(2, 2);
    uint32_t self = createUnitAt(*m_stateMan, pos, 100);

    // Even though it's overlapping (same position), passing self id should ignore it.
    float penalty = m_pathService->getSeparationPenalty(pos, self, 100);
    EXPECT_NEAR(penalty, 0.0f, 1e-3f);
}

// 3) Single unit collision separation penalty: verify numeric overlap calculation.
TEST_F(PathServiceTest, GetSeparationPenalty_SingleUnitCollision_CorrectOverlap)
{
    Feet pos = tileCenterFeet(3, 3);

    // Place other unit 120 feet to the right => dist = 120
    Feet otherPos = pos + Feet(120.0f, 0.0f);
    uint32_t other = createUnitAt(*m_stateMan, otherPos, 100);

    // separationRadius = 200, overlap = 200 - 120 = 80
    float penalty = m_pathService->getSeparationPenalty(pos, entt::null, 100);
    EXPECT_NEAR(penalty, 80.0f, 1e-3f);
}

// 4) Multiple unit collision separation penalty: sum overlaps from multiple units.
TEST_F(PathServiceTest, GetSeparationPenalty_MultipleUnits_SumsOverlaps)
{
    Feet pos = tileCenterFeet(4, 4);

    // Unit A: 130 ft right => overlapA = 200 - 130 = 70
    Feet aPos = pos + Feet(130.0f, 0.0f);
    createUnitAt(*m_stateMan, aPos, 100);

    // Unit B: 180 ft up => overlapB = 200 - 180 = 20
    Feet bPos = pos + Feet(0.0f, -180.0f);
    createUnitAt(*m_stateMan, bPos, 100);

    float penalty = m_pathService->getSeparationPenalty(pos, entt::null, 100);
    EXPECT_NEAR(penalty, 70.0f + 20.0f, 1e-3f);
}


} // namespace core
