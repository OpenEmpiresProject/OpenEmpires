#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "EventListener.h"
#include "GraphicInstruction.h"

#include <readerwriterqueue.h>
#include <vector>

namespace aion
{
using ThreadQueue = moodycamel::ReaderWriterQueue<std::vector<GraphicInstruction>>;

class Simulator : public EventListener
{
  public:
    Simulator(ThreadQueue &rendererQueue) : rendererQueue(rendererQueue) {};
    ~Simulator() = default;

  private:
    void onInit();
    void onExit();
    void onEvent(const Event &e);

    void onTick();
    void sendGraphicsInstructions();
    void simulatePhysics();

    ThreadQueue &rendererQueue;
};
} // namespace aion

#endif