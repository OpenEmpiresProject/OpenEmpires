#include "VisionSystem.h"

#include "FogOfWar.h"
#include "components/CompTransform.h"
#include "debug.h"
#include "logging/Logger.h"

using namespace core;

VisionSystem::VisionSystem()
{
    registerCallback(Event::Type::TICK, this, &VisionSystem::onTick);
    registerCallback(Event::Type::TRACKING_REQUEST, this, &VisionSystem::onTrackingRequest);
}

VisionSystem::~VisionSystem()
{
    // destructor
}

uint32_t getCompositeId(uint32_t tracker, uint32_t target)
{
    return tracker * Constants::MAX_ENTITIES + target;
}

bool isInLOSRoundedSquare(const Feet& p, const Rect<float>& b, float LOS)
{
    // Expanded LOS rectangle
    float left = b.x - LOS;
    float right = b.x + b.w + LOS;
    float top = b.y - LOS;
    float bottom = b.y + b.h + LOS;

    // Quick reject
    if (p.x < left || p.x > right || p.y < top || p.y > bottom)
        return false;

    // Axis-aligned inner rectangle (straight LOS zones)
    // If point is horizontally aligned with building
    if (p.x >= b.x && p.x <= b.x + b.w)
        return true;

    // If point is vertically aligned with building
    if (p.y >= b.y && p.y <= b.y + b.h)
        return true;

    // Corner checks: quarter circles of radius LOS
    auto sq = [](float v) { return v * v; };

    // top-left
    float dx = p.x - b.x;
    float dy = p.y - b.y;
    if (dx < 0 && dy < 0 && sq(dx) + sq(dy) <= LOS * LOS)
        return true;

    // top-right
    dx = p.x - (b.x + b.w);
    dy = p.y - b.y;
    if (dx > 0 && dy < 0 && sq(dx) + sq(dy) <= LOS * LOS)
        return true;

    // bottom-left
    dx = p.x - b.x;
    dy = p.y - (b.y + b.h);
    if (dx < 0 && dy > 0 && sq(dx) + sq(dy) <= LOS * LOS)
        return true;

    // bottom-right
    dx = p.x - (b.x + b.w);
    dy = p.y - (b.y + b.h);
    if (dx > 0 && dy > 0 && sq(dx) + sq(dy) <= LOS * LOS)
        return true;

    return false;
}

bool VisionSystem::isInLOS(const Tracking& trackerData, const Target& target)
{
    CompVision& vision = trackerData.trackerVision;

    if (vision.lineOfSightShape == LineOfSightShape::CIRCLE)
    {
        CompTransform& targetTransform = target.transform;
        auto& trackerPosition = trackerData.trackingRequest.center;

        float sqDistance = trackerPosition.distanceSquared(targetTransform.position);
        return (vision.lineOfSight * vision.lineOfSight) > sqDistance;
    }
    else if (vision.lineOfSightShape == LineOfSightShape::ROUNDED_SQUARE)
    {
        CompTransform& targetTransform = target.transform;
        CompVision& trackerVision = trackerData.trackerVision;
        auto cornersRect = CompBuilding::getLandInFeetRect(trackerData.trackingRequest.landArea);

        return isInLOSRoundedSquare(targetTransform.position, cornersRect,
                                    trackerVision.lineOfSight);
    }
    return false;
}

void VisionSystem::onEntityEnter(uint32_t entity, const Tile& tile, MapLayerType layer)
{
    const uint32_t target = entity;
    // spdlog::debug("Entity {} entered to tile {}", entity, tile.toString());

    const auto& trackers = m_trackersTileMap.getEntities(MapLayerType::STATIC, tile);

    for (auto tracker : trackers)
    {
        auto it = m_possibleTargetsByTracker.find(tracker);

        if (it == m_possibleTargetsByTracker.end())
        {
            auto& trackingRequest = m_trackingRequests[tracker];
            auto& trackerVision = m_stateMan->getComponent<CompVision>(tracker);
            Tracking tracking(trackerVision, trackingRequest);
            tracking.targets.emplace(target, Target(target, m_stateMan));
            m_possibleTargetsByTracker.emplace(tracker, tracking);
        }
        else
        {
            auto& tracking = it->second;
            tracking.targets.try_emplace(target, Target(target, m_stateMan));
        }
    }
}

void VisionSystem::onEntityExit(uint32_t entity, const Tile& tile, MapLayerType layer)
{
    const auto& trackers = m_trackersTileMap.getEntities(MapLayerType::STATIC, tile);

    for (auto tracker : trackers)
    {
        auto it = m_possibleTargetsByTracker.find(tracker);
        if (it != m_possibleTargetsByTracker.end())
        {
            auto& tracking = it->second;
            auto targetIt = tracking.targets.find(entity);

            if (targetIt != tracking.targets.end())
            {
                auto& target = targetIt->second;

                if (not isInLOS(tracking, target))
                {
                    auto& tracking = it->second;
                    tracking.targets.erase(target.entity);
                }
            }
        }
    }
}

void VisionSystem::onInit(EventLoop& eventLoop)
{
    m_stateMan = ServiceRegistry::getInstance().getService<StateManager>();
    auto& tileMap = m_stateMan->gameMap();
    tileMap.registerListner(shared_from_this());

    m_trackersTileMap.init(tileMap.width, tileMap.height);
}

void VisionSystem::onTick(const Event& e)
{
    for (const auto& [tracker, tracking] : m_possibleTargetsByTracker)
    {
        for (const auto& [_, target] : tracking.targets)
        {
            auto compositeId = getCompositeId(tracker, target.entity);
            auto it = m_activelyTrackedTargets.find(compositeId);

            if (isInLOS(tracking, target))
            {
                if (it == m_activelyTrackedTargets.end()) // Not actively tracked, but now in LOS
                {
                    spdlog::debug("Start tracking {} by tracker {}", target.entity, tracker);
                    m_activelyTrackedTargets.insert(compositeId);

                    LineOfSightData data;
                    data.tracker = tracker;
                    data.target = target.entity;
                    publishEvent(Event::Type::WITHIN_LINE_OF_SIGHT, data);
                }
            }
            else
            {
                if (it != m_activelyTrackedTargets.end()) // Actively tracked, but now out of LOS
                {
                    spdlog::debug("Stop tracking {} by tracker {}", target.entity, tracker);
                    m_activelyTrackedTargets.erase(compositeId);

                    LineOfSightData data;
                    data.tracker = tracker;
                    data.target = target.entity;
                    publishEvent(Event::Type::OUT_OF_LINE_OF_SIGHT, data);
                }
            }
        }
    }
}

void VisionSystem::onTrackingRequest(const Event& e)
{
    const auto& data = e.getData<TrackingRequestData>();

    auto& vision = m_stateMan->getComponent<CompVision>(data.entity);

    debug_assert(vision.activeTracking,
                 "Tracking request received for entity {} with disabled activeTracking",
                 data.entity);

    if (vision.activeTracking)
    {
        spdlog::debug("A tracking request for entity {}", data.entity);

        m_trackingRequests[data.entity] = data;

        if (vision.lineOfSightShape == LineOfSightShape::ROUNDED_SQUARE)
        {
            debug_assert(data.landArea.tiles.empty() == false,
                         "Land area cannot be empty for rounded square shaped LOS");
            FogOfWar::markRadius(
                data.landArea, vision.lineOfSight / Constants::FEET_PER_TILE,
                m_trackersTileMap.getDimensions(),
                [&](const Tile& t)
                {
                    spdlog::debug("{} tile is being watched by {}", t.toString(), data.entity);
                    m_trackersTileMap.addEntity(MapLayerType::STATIC, t, data.entity);
                });
        }
        else if (vision.lineOfSightShape == LineOfSightShape::CIRCLE)
        {
            debug_assert(data.center.isNull() == false,
                         "Entity center position cannot be null to enable visibility");
            FogOfWar::markRadius(
                data.center.toTile(), vision.lineOfSight / Constants::FEET_PER_TILE,
                m_trackersTileMap.getDimensions(), [&](const Tile& t)
                { m_trackersTileMap.addEntity(MapLayerType::STATIC, t, data.entity); });
        }
        else
        {
            spdlog::error("Unknown LOS shape for entity {}", data.entity);
        }
    }
    else
    {
        spdlog::error("Request to enable tracking with activeTracking disabled for entity {}",
                      data.entity);
    }
}

VisionSystem::Target::Target(uint32_t entity, Ref<StateManager> stateMan)
    : entity(entity), info(stateMan->getComponent<CompEntityInfo>(entity)),
      transform(stateMan->getComponent<CompTransform>(entity))
{
}
