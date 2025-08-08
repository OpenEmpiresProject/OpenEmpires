#ifndef HUD_H
#define HUD_H

#include "EventHandler.h"
#include "UI.h"
#include "UnitSelection.h"
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

  private:
    core::Ref<core::ui::Label> m_woodLabel;
    core::Ref<core::ui::Label> m_stoneabel;
    core::Ref<core::ui::Label> m_goldLabel;
    core::Ref<core::ui::Label> m_playerIdLabel;
    core::Ref<core::ui::Label> m_selectedIcon;
    core::Ref<core::ui::Label> m_constructionTextLabel;
    core::Ref<core::ui::Label> m_constructionProgressBarLabel;
    core::UnitSelection m_currentSelection;
};

} // namespace game

#endif