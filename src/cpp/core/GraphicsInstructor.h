#ifndef GRAPHICS_INSTRUCTOR_H
#define GRAPHICS_INSTRUCTOR_H

#include "EventHandler.h"
#include "FrameData.h"
#include "ThreadSynchronizer.h"

namespace core
{
class Coordinates;
class CompGraphics;

class GraphicsInstructor : public EventHandler
{
  public:
    GraphicsInstructor(ThreadSynchronizer<FrameData>& synchronizer);

  private:
    void onTick(const Event& e);

    void onTickStart();
    void onTickEnd();

    void sendGraphicsInstructions();
    void sendGraphiInstruction(CompGraphics* instruction);
    void updateGraphicComponents();

  private:
    std::shared_ptr<Coordinates> m_coordinates;
    ThreadSynchronizer<FrameData>& m_synchronizer;
    uint32_t m_frameCount = 0;
    bool m_initialized = false;
};
} // namespace core

#endif