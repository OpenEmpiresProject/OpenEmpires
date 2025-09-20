#ifndef HUD_H
#define HUD_H

#include "EntitySelection.h"
#include "EventHandler.h"
#include "UI.h"
#include "utils/Types.h"
#include "PlayerController.h"

namespace game
{
class HUDUpdater : public core::EventHandler
{
  public:
    HUDUpdater();
    ~HUDUpdater();

  private:
    void onTick(const core::Event& e);
    void updateResourcePanel();
    void updateProgressBar();
    void updatePlayerControllerRef();
    void updateLabelRef(core::Ref<core::ui::Label>& label, const std::string& text);
    void onUnitSelection(const core::Event& e);

  private:
    core::Ref<core::ui::Label> m_woodLabel;
    core::Ref<core::ui::Label> m_stoneabel;
    core::Ref<core::ui::Label> m_goldLabel;
    core::Ref<core::ui::Label> m_playerIdLabel;
    core::Ref<core::ui::Label> m_selectedName;
    core::Ref<core::ui::Label> m_selectedIcon;
    core::Ref<core::ui::Label> m_progressTextLabel;
    core::Ref<core::ui::Label> m_progressItemNameLabel;
    core::Ref<core::ui::Label> m_unitInProgressIcon;
    core::Ref<core::ui::Label> m_progressBarLabel;
    core::Ref<core::ui::Label> m_queuedUnitIcons[core::Constants::ABSOLUTE_MAX_UNIT_QUEUE_SIZE];
    core::EntitySelection m_currentSelection;
    core::Ref<core::PlayerController> m_playerController;
};

} // namespace game

#endif