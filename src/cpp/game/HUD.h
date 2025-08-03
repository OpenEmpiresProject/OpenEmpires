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
    void updateLabelRef(ion::Ref<ion::ui::Label>& label, const std::string& text);
    void onUnitSelection(const ion::Event& e);

    ion::Ref<ion::ui::Label> m_woodLabel;
    ion::Ref<ion::ui::Label> m_stoneabel;
    ion::Ref<ion::ui::Label> m_goldLabel;
    ion::Ref<ion::ui::Label> m_playerIdLabel;
    ion::Ref<ion::ui::Label> m_selectedIcon;
};

} // namespace game

#endif