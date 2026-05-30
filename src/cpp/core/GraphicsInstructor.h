#ifndef GRAPHICS_INSTRUCTOR_H
#define GRAPHICS_INSTRUCTOR_H

#include "Coordinates.h"
#include "EventHandler.h"
#include "FrameData.h"
#include "HumanController.h"
#include "Settings.h"
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
    bool onTick(const Event& e);
    bool onMouseMove(const Event& e);
    bool onMouseButtonUp(const Event& e);
    bool onMouseButtonDown(const Event& e);

    void onTickStart();
    void onTickEnd();

    void sendGraphicsInstructions();
    void sendGraphiInstruction(CompGraphics* instruction);
    void updateGraphicComponents();

  private:
    LazyServiceRef<Coordinates> m_coordinates;
    LazyServiceRef<HumanController> m_playerController;
    LazyServiceRef<StateManager> m_stateManager;
    LazyServiceRef<Settings> m_settings;
    ThreadSynchronizer<FrameData>& m_synchronizer;
    uint32_t m_frameCount = 0;
    bool m_initialized = false;
};
} // namespace core

#endif