#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "EventHandler.h"
#include "GraphicInstruction.h"
#include "ThreadQueue.h"

#include <readerwriterqueue.h>
#include <vector>

namespace aion
{
class Simulator : public EventHandler
{
  public:
    Simulator(ThreadQueue& rendererQueue) : rendererQueue(rendererQueue) {};
    ~Simulator() = default;

  private:
    void onInit(EventLoop* eventLoop);
    void onExit();
    void onEvent(const Event& e);

    void onTick();
    void sendGraphicsInstructions();
    void sendStaticInstructions(); // Static from ticks point of view, but not entire system
    void simulatePhysics();

    ThreadQueue& rendererQueue;
    bool sentStaticInstructions = false;
};
} // namespace aion

#endif