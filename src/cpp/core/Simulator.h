#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "Coordinates.h"
#include "EventHandler.h"
#include "EventPublisher.h"
#include "FrameData.h"
#include "ThreadQueue.h"
#include "ThreadSynchronizer.h"
#include "TileMap.h"
#include "UnitSelection.h"
#include "commands/CmdMove.h"
#include "components/CompBuilding.h"
#include "components/CompGraphics.h"

namespace core
{
class Simulator : public EventHandler
{
  public:
    Simulator(ThreadSynchronizer<FrameData>& synchronizer,
              std::shared_ptr<EventPublisher> publisher);
    ~Simulator() = default;

  private:
    // EventHandler overrides
    void onInit(EventLoop& eventLoop) override;

    void onTick(const Event& e);
    void onKeyUp(const Event& e);
    void onKeyDown(const Event& e);

    void onTickStart();
    void onTickEnd();
    void onSynchorizedBlock();
    void onUnitSelection(const Event& e);

    void sendGraphicsInstructions();
    void sendGraphiInstruction(CompGraphics* instruction);
    void updateGraphicComponents();

    std::shared_ptr<Coordinates> m_coordinates;
    ThreadSynchronizer<FrameData>& m_synchronizer;
    int m_frame = 0;

    std::shared_ptr<EventPublisher> m_publisher;
    bool m_initialized = false;
    bool m_showSpamLogs = false;
};
} // namespace core

#endif