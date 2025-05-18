#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "Coordinates.h"
#include "EventHandler.h"
#include "EventPublisher.h"
#include "FrameData.h"
#include "ThreadQueue.h"
#include "ThreadSynchronizer.h"
#include "UnitSelection.h"
#include "components/CompGraphics.h"

namespace aion
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
    void onExit() override;
    void onEvent(const Event& e) override;

    void onTick();
    void onTickStart();
    void onTickEnd();
    void sendGraphicsInstructions();
    void sendGraphiInstruction(CompGraphics* instruction);
    void testPathFinding(const Vec2d& end);
    void updateGraphicComponents();
    void onSelectingUnits(const Vec2d& startScreenPos, const Vec2d& endScreenPos);
    void resolveAction(const Vec2d& targetFeetPos);
    void testBuildMill(const Vec2d& targetFeetPos, int buildingType, Size size);

    Coordinates m_coordinates;
    ThreadSynchronizer<FrameData>& m_synchronizer;
    int m_frame = 0;
    Vec2d m_selectionStartPosScreenUnits;
    Vec2d m_selectionEndPosScreenUnits;
    bool m_isSelecting = false;
    std::shared_ptr<EventPublisher> m_publisher;
    UnitSelection m_currentUnitSelection;
    Vec2d m_lastMouseScreenPos;
    uint32_t m_currentBuildingOnPlacement = 0;
    bool m_initialized = false;
};
} // namespace aion

#endif