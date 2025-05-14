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

#include <readerwriterqueue.h>
#include <vector>

namespace aion
{
class Simulator : public EventHandler
{
  public:
    Simulator(ThreadSynchronizer<FrameData>& synchronizer,
              std::shared_ptr<EventPublisher> publisher);
    ~Simulator() = default;

  private:
    void onInit(EventLoop* eventLoop);
    void onExit();
    void onEvent(const Event& e);

    void onTick();
    void sendGraphicsInstructions();
    void sendGraphiInstruction(CompGraphics* instruction);
    void testPathFinding(const Vec2d& end);
    void updateGraphicComponents();
    void incrementDirtyVersion();
    void onSelectingUnits(const Vec2d& startScreenPos, const Vec2d& endScreenPos);
    void resolveAction(const Vec2d& targetFeetPos);

    Coordinates m_coordinates;

    ThreadSynchronizer<FrameData>& m_synchronizer;
    int m_frame = 0;

    Vec2d m_selectionStartPosScreenUnits;
    Vec2d m_selectionEndPosScreenUnits;
    bool m_isSelecting = false;
    std::shared_ptr<EventPublisher> m_publisher;

    UnitSelection m_currentUnitSelection;
};
} // namespace aion

#endif