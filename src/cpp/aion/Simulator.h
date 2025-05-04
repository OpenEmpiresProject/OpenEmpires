#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "EventHandler.h"
#include "GraphicInstruction.h"
#include "ThreadQueue.h"
#include "Viewport.h"

#include <readerwriterqueue.h>
#include <vector>

namespace aion
{
class Simulator : public EventHandler
{
  public:
    Simulator(ThreadQueue& rendererQueue, Viewport& viewport)
        : rendererQueue(rendererQueue), viewport(viewport)
    {
    }
    ~Simulator() = default;

  private:
    void onInit(EventLoop* eventLoop);
    void onExit();
    void onEvent(const Event& e);

    void onTick();
    void sendGraphicsInstructions();
    void sendStaticInstructions(); // Static from ticks point of view, but not entire system
    void simulatePhysics();
    void sendStaticTileInstructions();
    void sendInitialUnitsInstructions();
    void sendGraphiInstruction(GraphicInstruction* instruction);
    void testPathFinding(const Vec2d& start, const Vec2d& end);

    ThreadQueue& rendererQueue;
    bool sentStaticInstructions = false;

    Vec2d startPosition{0, 0};
    Viewport& viewport;

    ThreadMessage* messageToRenderer = nullptr;
};
} // namespace aion

#endif