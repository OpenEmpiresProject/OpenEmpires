#ifndef HUD_H
#define HUD_H

#include "EventHandler.h"
#include "UI.h"
#include "utils/Types.h"

namespace game
{
class HUD : public ion::EventHandler
{
  public:
    HUD(/* args */);
    ~HUD();

  private:
    void onTick(const ion::Event& e);

    ion::Ref<ion::ui::Label> m_woodLabel;
};

} // namespace game

#endif