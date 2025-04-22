#ifndef INPUTLISTENER_H
#define INPUTLISTENER_H

#include "EventHandler.h"
#include "GameSettings.h"
#include "ThreadQueue.h"
#include "Viewport.h"

namespace aion
{
class InputListener : public EventHandler
{
  public:
    InputListener(Viewport& viewport, const GameSettings& settings)
        : viewport(viewport), settings(settings)
    {
    }
    virtual ~InputListener() = default;

  private:
    void onInit(EventLoop* eventLoop) override;
    void onExit() override;

    void onEvent(const Event& e) override;
    void handleInputEvents();

    EventLoop* eventLoop = nullptr;
    Viewport& viewport;
    const GameSettings& settings;
};
} // namespace aion

#endif