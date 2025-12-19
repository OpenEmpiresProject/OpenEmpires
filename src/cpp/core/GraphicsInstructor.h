#ifndef GRAPHICS_INSTRUCTOR_H
#define GRAPHICS_INSTRUCTOR_H

#include "Coordinates.h"
#include "EventHandler.h"
#include "FrameData.h"
#include "PlayerController.h"
#include "ThreadSynchronizer.h"
#include "utils/LazyServiceRef.h"

namespace core
{
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
    LazyServiceRef<Coordinates> m_coordinates;
    LazyServiceRef<PlayerController> m_playerController;
    LazyServiceRef<StateManager> m_stateManager;
    ThreadSynchronizer<FrameData>& m_synchronizer;
    uint32_t m_frameCount = 0;
    bool m_initialized = false;
};
} // namespace core

#endif