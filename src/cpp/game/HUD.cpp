#include "HUD.h"

#include "GameTypes.h"
#include "PlayerManager.h"
#include "ServiceRegistry.h"
#include "UIManager.h"
#include "utils/Logger.h"

using namespace game;
using namespace ion;

HUD::HUD(/* args */)
{
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
            if (window->name == "resourcePanel")
            {
                for (auto child : window->children)
                {
                    if (child->name == text)
                    {
                        label = std::static_pointer_cast<ui::Label>(child);
                    }
                }
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
        m_woodLabel->text = std::to_string(player->getResourceAmount(ResourceType::WOOD));
    }
    else
    {
        spdlog::error("Could not find wood label in resource panel window.");
    }

    if (m_stoneabel != nullptr)
    {
        auto playerManager = ServiceRegistry::getInstance().getService<PlayerManager>();
        auto player = playerManager->getViewingPlayer();
        m_stoneabel->text = std::to_string(player->getResourceAmount(ResourceType::STONE));
    }
    else
    {
        spdlog::error("Could not find stone label in resource panel window.");
    }

    if (m_goldLabel != nullptr)
    {
        auto playerManager = ServiceRegistry::getInstance().getService<PlayerManager>();
        auto player = playerManager->getViewingPlayer();
        m_goldLabel->text = std::to_string(player->getResourceAmount(ResourceType::GOLD));
    }
    else
    {
        spdlog::error("Could not find gold label in resource panel window.");
    }

    if (m_playerIdLabel != nullptr)
    {
        auto playerManager = ServiceRegistry::getInstance().getService<PlayerManager>();
        auto player = playerManager->getViewingPlayer();
        m_playerIdLabel->text = "Player " + std::to_string(player->getId());
    }
    else
    {
        spdlog::error("Could not find player label in resource panel window.");
    }
}
