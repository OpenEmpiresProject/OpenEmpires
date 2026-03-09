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
} // namespace core
