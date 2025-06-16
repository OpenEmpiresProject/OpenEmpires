#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "Coordinates.h"
#include "EventHandler.h"
#include "EventPublisher.h"
#include "FrameData.h"
#include "ThreadQueue.h"
#include "ThreadSynchronizer.h"
#include "UnitSelection.h"
#include "commands/CmdMove.h"
#include "components/CompBuilding.h"
#include "components/CompGraphics.h"

namespace ion
{
class Simulator : public EventHandler
{
  public:
    Simulator(ThreadSynchronizer<FrameData>& synchronizer,
              std::shared_ptr<EventPublisher> publisher);
    ~Simulator() = default;

  private:
    // EventHandler overrides
    void onInit(EventLoop* eventLoop) override;

    void onTick(const Event& e);
    void onKeyUp(const Event& e);
    void onKeyDown(const Event& e);
    void onMouseButtonUp(const Event& e);
    void onMouseButtonDown(const Event& e);
    void onMouseMove(const Event& e);

    void onTickStart();
    void onTickEnd();
    void onSynchorizedBlock();
    void onUnitSelection(const Event& e);

    void sendGraphicsInstructions();
    void sendGraphiInstruction(CompGraphics* instruction);
    void updateGraphicComponents();
    void onSelectingUnits(const Vec2d& startScreenPos, const Vec2d& endScreenPos);
    void onClickToSelect(const Vec2d& screenPos);
    void resolveSelection(const Vec2d& screenPos);
    void resolveAction(const Vec2d& screenPos);
    void testBuild(const Vec2d& targetFeetPos, int buildingType, Size size);
    bool canPlaceBuildingAt(const CompBuilding& building, const Vec2d& feet, bool& outOfMap);
    void addEntitiesToSelection(const std::vector<uint32_t>& selectedEntities);
    void clearSelection();
    uint32_t whatIsAt(const Vec2d& screenPos);

    std::shared_ptr<Coordinates> m_coordinates;
    ThreadSynchronizer<FrameData>& m_synchronizer;
    int m_frame = 0;
    Vec2d m_selectionStartPosScreenUnits;
    bool m_isSelecting = false;
    std::shared_ptr<EventPublisher> m_publisher;
    UnitSelection m_currentUnitSelection;
    Vec2d m_lastMouseScreenPos;
    uint32_t m_currentBuildingOnPlacement = 0;
    bool m_initialized = false;
};
} // namespace ion

#endif