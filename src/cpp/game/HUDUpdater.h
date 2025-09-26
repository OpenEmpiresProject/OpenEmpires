#ifndef HUD_H
#define HUD_H

#include "EntitySelection.h"
#include "EntityTypeRegistry.h"
#include "EventHandler.h"
#include "PlayerController.h"
#include "ServiceRegistry.h"
#include "UI.h"
#include "UIManager.h"
#include "utils/Logger.h"
#include "utils/Types.h"

namespace game
{
class HUDUpdater : public core::EventHandler
{
  public:
    HUDUpdater();
    ~HUDUpdater() = default;

  private:
    void onTick(const core::Event& e);

  private:
    void updateUIElementReferences();
    void updateResourcePanel();
    void updateProgressBar();
    void updateFactoryUnitCreations(uint32_t factoryEntity);
    void updateBuildingConstruction(core::CompBuilding& building, core::CompEntityInfo& info);

    template <typename T> void updateUIElementRef(core::Ref<T>& elementRef, const std::string& text)
    {
        if (elementRef == nullptr)
        {
            auto uiManager = core::ServiceRegistry::getInstance().getService<core::UIManager>();
            auto& windows = uiManager->getWindows();
            for (auto& window : windows)
            {
                auto child = window->findChild(text);
                if (child != nullptr)
                {
                    elementRef = std::static_pointer_cast<T>(child);
                    return;
                }
            }
            spdlog::error("Could not find {} element", text);
        }
    }
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
    core::Ref<core::ui::Widget> m_creationInProgressGroup;
    core::Ref<core::ui::Widget> m_creationQueueGroup;
    core::Ref<core::GameState> m_gameState;
    core::Ref<core::EntityTypeRegistry> m_typeRegistry;
};

} // namespace game

#endif