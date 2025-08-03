#include "HUD.h"

#include "GameTypes.h"
#include "PlayerManager.h"
#include "ServiceRegistry.h"
#include "UIManager.h"
#include "components/CompSelectible.h"
#include "utils/Logger.h"

using namespace game;
using namespace ion;

HUD::HUD(/* args */)
{
    registerCallback(Event::Type::UNIT_SELECTION, this, &HUD::onUnitSelection);
    registerCallback(Event::Type::TICK, this, &HUD::onTick);
}

HUD::~HUD()
{
}

void HUD::updateLabelRef(Ref<ion::ui::Label>& label, const std::string& text)
{
    if (label == nullptr)
    {
        auto uiManager = ServiceRegistry::getInstance().getService<UIManager>();
        auto& windows = uiManager->getWindows();
        for (auto window : windows)
        {
            auto child = window->findChild(text);
            if (child != nullptr)
            {
                label = std::static_pointer_cast<ui::Label>(child);
                break;
            }
        }
    }
}

void HUD::onTick(const Event& e)
{
    updateLabelRef(m_woodLabel, "wood");
    updateLabelRef(m_stoneabel, "stone");
    updateLabelRef(m_goldLabel, "gold");
    updateLabelRef(m_playerIdLabel, "player");

    if (m_woodLabel != nullptr)
    {
        auto playerManager = ServiceRegistry::getInstance().getService<PlayerManager>();
        auto player = playerManager->getViewingPlayer();
        m_woodLabel->setText(std::to_string(player->getResourceAmount(ResourceType::WOOD)));
    }
    else
    {
        spdlog::error("Could not find wood label in resource panel window.");
    }

    if (m_stoneabel != nullptr)
    {
        auto playerManager = ServiceRegistry::getInstance().getService<PlayerManager>();
        auto player = playerManager->getViewingPlayer();
        m_stoneabel->setText(std::to_string(player->getResourceAmount(ResourceType::STONE)));
    }
    else
    {
        spdlog::error("Could not find stone label in resource panel window.");
    }

    if (m_goldLabel != nullptr)
    {
        auto playerManager = ServiceRegistry::getInstance().getService<PlayerManager>();
        auto player = playerManager->getViewingPlayer();
        m_goldLabel->setText(std::to_string(player->getResourceAmount(ResourceType::GOLD)));
    }
    else
    {
        spdlog::error("Could not find gold label in resource panel window.");
    }

    if (m_playerIdLabel != nullptr)
    {
        auto playerManager = ServiceRegistry::getInstance().getService<PlayerManager>();
        auto player = playerManager->getViewingPlayer();
        m_playerIdLabel->setText("Player " + std::to_string(player->getId()));
    }
    else
    {
        spdlog::error("Could not find player label in resource panel window.");
    }
}

void HUD::onUnitSelection(const Event& e)
{
    updateLabelRef(m_selectedIcon, "selected_icon");
    m_selectedIcon->setVisible(false);

    auto selectionData = e.getData<UnitSelectionData>();
    auto gameState = ServiceRegistry::getInstance().getService<GameState>();

    for (auto unit : selectionData.selection.selectedEntities)
    {
        auto comSelectible = gameState->getComponent<CompSelectible>(unit);
        if (m_selectedIcon != nullptr)
        {
            m_selectedIcon->setVisible(true);
            m_selectedIcon->setBackgroundImage(comSelectible.icon);
        }
        else
        {
            spdlog::error("Could not find selected-icon label in info panel window.");
        }
        break;
    }
}
