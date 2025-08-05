#ifndef PLAYERACTIONRESOLVER_H
#define PLAYERACTIONRESOLVER_H

#include "Event.h"
#include "EventHandler.h"
#include "GameTypes.h"
#include "utils/Types.h"

namespace game
{
class PlayerActionResolver : public core::EventHandler
{
  public:
    PlayerActionResolver(/* args */);
    ~PlayerActionResolver();

  private:
    core::Vec2 m_lastMouseScreenPos;

    void onEvent(const core::Event& e) override;
    void onKeyUp(const core::Event& e);
    void onMouseMove(const core::Event& e);
};

} // namespace game

#endif