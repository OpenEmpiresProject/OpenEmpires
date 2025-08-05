#ifndef HUD_H
#define HUD_H

#include "EventHandler.h"
#include "UI.h"
#include "utils/Types.h"

namespace game
{
class HUD : public core::EventHandler
{
  public:
    HUD(/* args */);
    ~HUD();

  private:
    void onTick(const core::Event& e);
    void updateLabelRef(core::Ref<core::ui::Label>& label, const std::string& text);
    void onUnitSelection(const core::Event& e);

    core::Ref<core::ui::Label> m_woodLabel;
    core::Ref<core::ui::Label> m_stoneabel;
    core::Ref<core::ui::Label> m_goldLabel;
    core::Ref<core::ui::Label> m_playerIdLabel;
    core::Ref<core::ui::Label> m_selectedIcon;
};

} // namespace game

#endif