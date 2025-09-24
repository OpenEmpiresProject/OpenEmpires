#include "HUDUpdater.h"

#include "GameTypes.h"
#include "GraphicsRegistry.h"
#include "PlayerController.h"
#include "PlayerFactory.h"
#include "components/CompBuilding.h"
#include "components/CompEntityInfo.h"
#include "components/CompSelectible.h"
#include "components/CompUnitFactory.h"
#include "EntityTypeRegistry.h"

using namespace game;
using namespace core;

HUDUpdater::HUDUpdater()
{
    registerCallback(Event::Type::ENTITY_SELECTION, this, &HUDUpdater::onUnitSelection);
    registerCallback(Event::Type::TICK, this, &HUDUpdater::onTick);
}

HUDUpdater::~HUDUpdater()
{
}

void HUDUpdater::onTick(const Event& e)
{
    // Late binding label pointers
    updateUIElementRef(m_woodLabel, "wood");
    updateUIElementRef(m_stoneabel, "stone");
    updateUIElementRef(m_goldLabel, "gold");
    updateUIElementRef(m_playerIdLabel, "player");
    updateUIElementRef(m_progressTextLabel, "progress_label");
    updateUIElementRef(m_progressItemNameLabel, "progress_item_name");
    updateUIElementRef(m_progressBarLabel, "progress_bar_label");
    updateUIElementRef(m_unitInProgressIcon, "unit_creating_icon");
    updateUIElementRef(m_creationInProgressGroup, "creation_in_progress_group");
    updateUIElementRef(m_creationQueueGroup, "creation_queue_group");
    updatePlayerControllerRef();

    for (int i = 0; i < Constants::ABSOLUTE_MAX_UNIT_QUEUE_SIZE; ++i)
    {
        updateUIElementRef(m_queuedUnitIcons[i], fmt::format("queued_unit_icon_{}", i));
    }

    updateResourcePanel();
    updateProgressBar();
}

void HUDUpdater::onUnitSelection(const Event& e)
{
    updateUIElementRef(m_selectedIcon, "selected_icon");
    updateUIElementRef(m_selectedName, "selected_name");
    m_selectedIcon->setVisible(false);
    m_selectedName->setVisible(false);
    m_creationInProgressGroup->hide();
    m_creationQueueGroup->hide();

    m_currentSelection = e.getData<EntitySelectionData>().selection;
    auto gameState = ServiceRegistry::getInstance().getService<GameState>();

    for (auto unit : m_currentSelection.selectedEntities)
    {
        auto comSelectible = gameState->getComponent<CompSelectible>(unit);
        if (m_selectedIcon != nullptr)
        {
            m_selectedIcon->setVisible(true);
            m_selectedIcon->setBackgroundImage(comSelectible.icon);
            m_selectedName->setVisible(true);
            m_selectedName->setText(comSelectible.displayName);
        }
        else
        {
            spdlog::error("Could not find selected-icon label in info panel window.");
        }
        break;
    }
}

void HUDUpdater::updateProgressBar()
{
    // It is not possible to display statuses of multiple buildings
    //
    if (m_currentSelection.selectedEntities.size() == 1)
    {
        auto gameState = ServiceRegistry::getInstance().getService<GameState>();
        auto entityInfoRegistry = ServiceRegistry::getInstance().getService<EntityTypeRegistry>();

        auto entity = m_currentSelection.selectedEntities[0];
        if (gameState->hasComponent<CompBuilding>(entity))
        {
            auto [building, buildingInfo] =
                gameState->getComponents<CompBuilding, CompEntityInfo>(entity);

            if (building.isConstructed())
            {
                m_creationInProgressGroup->hide();
                m_creationQueueGroup->hide();

                for (int i = 0; i < Constants::ABSOLUTE_MAX_UNIT_QUEUE_SIZE; ++i)
                {
                    m_queuedUnitIcons[i]->setVisible(false);
                }

                if (gameState->hasComponent<CompUnitFactory>(entity))
                {
                    auto& factory = gameState->getComponent<CompUnitFactory>(entity);

                    if (factory.productionQueue.empty() == false &&
                        factory.currentUnitProgress < 100)
                    {
                        auto displayName =
                            entityInfoRegistry->getHUDDisplayName(factory.productionQueue[0]);
                        auto unitIcon =
                            entityInfoRegistry->getHUDIcon(factory.productionQueue[0]);
                        m_progressTextLabel->setText(
                            std::format("Creating - {}%", (int) factory.currentUnitProgress));

                        m_progressItemNameLabel->setText(displayName);
                        m_unitInProgressIcon->setBackgroundImage(unitIcon);

                        auto graphic = m_progressBarLabel->getBackgroundImage();
                        graphic.variation = factory.currentUnitProgress;
                        m_progressBarLabel->setBackgroundImage(graphic);

                        m_creationInProgressGroup->show();
                    }

                    if (factory.productionQueue.empty() == false)
                    {
                        m_creationQueueGroup->show();
                    }

                    for (int i = 1; i < Constants::ABSOLUTE_MAX_UNIT_QUEUE_SIZE; ++i)
                    {
                        if (i < factory.productionQueue.size())
                        {
                            auto unitIcon =
                                entityInfoRegistry->getHUDIcon(factory.productionQueue[i]);
                            m_queuedUnitIcons[i - 1]->setBackgroundImage(unitIcon);
                            m_queuedUnitIcons[i - 1]->setVisible(true);
                        }
                        else
                        {
                            m_queuedUnitIcons[i - 1]->setVisible(false);
                        }
                    }
                }
            }
            else
            {
                auto displayName = entityInfoRegistry->getHUDDisplayName(buildingInfo.entityType);

                m_progressTextLabel->setText(
                    std::format("Building - {}%", building.constructionProgress));
                m_progressItemNameLabel->setText(displayName);
                auto graphic = m_progressBarLabel->getBackgroundImage();
                graphic.variation = building.constructionProgress;
                m_progressBarLabel->setBackgroundImage(graphic);

                m_creationInProgressGroup->show();
            }
        }
    }
}

void HUDUpdater::updatePlayerControllerRef()
{
    if (m_playerController == nullptr)
    {
        m_playerController = ServiceRegistry::getInstance().getService<PlayerController>();
    }
}

void HUDUpdater::updateResourcePanel()
{
    auto player = m_playerController->getPlayer();

    m_woodLabel->setText(std::to_string(player->getResourceAmount(ResourceType::WOOD)));
    m_stoneabel->setText(std::to_string(player->getResourceAmount(ResourceType::STONE)));
    m_goldLabel->setText(std::to_string(player->getResourceAmount(ResourceType::GOLD)));
    m_playerIdLabel->setText("Player " + std::to_string(player->getId()));
}
