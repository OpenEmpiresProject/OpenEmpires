#include "HUD.h"

#include "PlayerManager.h"
#include "ResourceTypes.h"
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

void HUD::onTick(const Event& e)
{
    if (m_woodLabel == nullptr)
    {
        auto uiManager = ServiceRegistry::getInstance().getService<UIManager>();
        auto& windows = uiManager->getWindows();
        for (auto window : windows)
        {
            if (window->name == "resourcePanel")
            {
                for (auto child : window->children)
                {
                    if (child->name == "wood")
                    {
                        m_woodLabel = std::static_pointer_cast<ui::Label>(child);
                    }
                }
            }
        }
    }

    if (m_woodLabel != nullptr)
    {
        auto playerManager = ServiceRegistry::getInstance().getService<PlayerManager>();
        auto player = playerManager->getPlayer(0); // TODO - replace with current player on the UI
        m_woodLabel->text = std::to_string(player->getResourceAmount(ResourceType::WOOD));
    }
    else
    {
        spdlog::error("Could not find wood label in resource panel window.");
    }
}
