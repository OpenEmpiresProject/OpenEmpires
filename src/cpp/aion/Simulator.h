#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "EventHandler.h"
#include "FrameData.h"
#include "ThreadQueue.h"
#include "ThreadSynchronizer.h"
#include "Viewport.h"
#include "components/CompGraphics.h"

#include <readerwriterqueue.h>
#include <vector>

namespace aion
{
class Simulator : public EventHandler
{
  public:
    Simulator(Viewport& viewport, ThreadSynchronizer<FrameData>& synchronizer)
        : m_viewport(viewport), m_synchronizer(synchronizer)
    {
    }
    ~Simulator() = default;

  private:
    void onInit(EventLoop* eventLoop);
    void onExit();
    void onEvent(const Event& e);

    void onTick();
    void sendGraphicsInstructions();
    void sendGraphiInstruction(CompGraphics* instruction);
    void testPathFinding(const Vec2d& start, const Vec2d& end);
    void updateGraphicComponents();
    void incrementDirtyVersion();

    Vec2d m_startPosition{0, 0};
    Viewport& m_viewport;

    ThreadSynchronizer<FrameData>& m_synchronizer;
    int m_frame = 0;
};
} // namespace aion

#endif