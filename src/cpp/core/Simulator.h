#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "EventHandler.h"
#include "FrameData.h"
#include "ThreadSynchronizer.h"

namespace core
{
class Coordinates;
class CompGraphics;

class Simulator : public EventHandler
{
  public:
    Simulator(ThreadSynchronizer<FrameData>& synchronizer);

  private:
    // EventHandler overrides
    void onInit(EventLoop& eventLoop) override;

    void onTick(const Event& e);
    void onKeyUp(const Event& e);

    void onTickStart();
    void onTickEnd();
    void onSynchorizedBlock();
    void onUnitSelection(const Event& e);

    void sendGraphicsInstructions();
    void sendGraphiInstruction(CompGraphics* instruction);
    void updateGraphicComponents();

  private:
    std::shared_ptr<Coordinates> m_coordinates;
    ThreadSynchronizer<FrameData>& m_synchronizer;
    int m_frame = 0;

    bool m_initialized = false;
    bool m_showSpamLogs = false;
};
} // namespace core

#endif