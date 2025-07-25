#ifndef PLAYERACTIONRESOLVER_H
#define PLAYERACTIONRESOLVER_H

#include "Event.h"
#include "EventHandler.h"
#include "GameTypes.h"
#include "utils/Types.h"

namespace game
{
class PlayerActionResolver : public ion::EventHandler
{
  public:
    PlayerActionResolver(/* args */);
    ~PlayerActionResolver();

  private:
    ion::Vec2 m_lastMouseScreenPos;

    void onEvent(const ion::Event& e) override;
    void onKeyUp(const ion::Event& e);
    void onMouseMove(const ion::Event& e);
};

} // namespace game

#endif