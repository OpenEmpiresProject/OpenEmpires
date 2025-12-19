#ifndef CORE_VISIONSYSTEM_H
#define CORE_VISIONSYSTEM_H
#include "EventHandler.h"
#include "TileMapListner.h"
#include "components/CompEntityInfo.h"
#include "components/CompTransform.h"
#include "components/CompVision.h"

#include <entt/entity/registry.hpp>

namespace core
{
class VisionSystem : public TileMapListner,
                     public EventHandler,
                     public std::enable_shared_from_this<VisionSystem>
{
  public:
    VisionSystem();
    ~VisionSystem();

  protected:
    void onEntityEnter(uint32_t entity, const Tile& tile, MapLayerType layer) override;
    void onEntityExit(uint32_t entity, const Tile& tile, MapLayerType layer) override;
    void onInit(EventLoop& eventLoop) override;
    void onTick(const Event& e);
    void onTrackingRequest(const Event& e);

    struct Target
    {
        uint32_t entity = entt::null;
        std::reference_wrapper<CompEntityInfo> info;
        std::reference_wrapper<CompTransform> transform;

        Target(uint32_t entity, Ref<StateManager> stateMan);
    };

    struct Tracking
    {
        std::reference_wrapper<CompVision> trackerVision;
        TrackingRequestData trackingRequest;
        std::unordered_map<uint32_t, Target> targets; // by target entity id

        Tracking(CompVision& trackerVision, const TrackingRequestData& request)
            : trackerVision(trackerVision), trackingRequest(request)
        {
        }
    };

    static bool isInLOS(const Tracking& trackerData, const Target& target);

    std::unordered_map<uint32_t, TrackingRequestData> m_trackingRequests;
    std::unordered_map<uint32_t, Tracking> m_possibleTargetsByTracker;
    std::unordered_set<uint32_t> m_activelyTrackedTargets;
    LazyServiceRef<StateManager> m_stateMan;
    TileMap m_trackersTileMap;
};
} // namespace core

#endif // CORE_VISIONSYSTEM_H
