#include "HUDUpdater.h"

#include "GameTypes.h"
#include "GraphicsRegistry.h"
#include "PlayerController.h"
#include "PlayerFactory.h"
#include "components/CompBuilding.h"
#include "components/CompEntityInfo.h"
#include "components/CompSelectible.h"
#include "components/CompUnitFactory.h"

using namespace game;
using namespace core;

HUDUpdater::HUDUpdater()
{
    registerCallback(Event::Type::ENTITY_SELECTION, this, &HUDUpdater::onUnitSelection);
    registerCallback(Event::Type::TICK, this, &HUDUpdater::onTick);

    m_gameState = ServiceRegistry::getInstance().getService<GameState>();
    m_typeRegistry = ServiceRegistry::getInstance().getService<EntityTypeRegistry>();
    m_playerController = ServiceRegistry::getInstance().getService<PlayerController>();
}

void HUDUpdater::onTick(const Event& e)
{
    updateUIElementReferences();
    updateResourcePanel();
    updateProgressBar();
}

void HUDUpdater::onUnitSelection(const Event& e)
{
    m_selectedIcon->setVisible(false);
    m_selectedName->setVisible(false);
    m_creationInProgressGroup->setVisible(false);
    m_creationQueueGroup->setVisible(false);

    m_currentSelection = e.getData<EntitySelectionData>().selection;

    for (auto unit : m_currentSelection.selectedEntities)
    {
        auto& comSelectible = m_gameState->getComponent<CompSelectible>(unit);
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
        auto entity = m_currentSelection.selectedEntities[0];
        if (m_gameState->hasComponent<CompBuilding>(entity))
        {
            auto [building, entityInfo] =
                m_gameState->getComponents<CompBuilding, CompEntityInfo>(entity);

            if (building.isConstructed())
            {
                updateFactoryUnitCreations(entity);
            }
            else
            {
                updateBuildingConstruction(building, entityInfo);
            }
        }
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

void HUDUpdater::updateUIElementReferences()
{
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
    updateUIElementRef(m_selectedIcon, "selected_icon");
    updateUIElementRef(m_selectedName, "selected_name");

    for (int i = 0; i < Constants::ABSOLUTE_MAX_UNIT_QUEUE_SIZE; ++i)
    {
        updateUIElementRef(m_queuedUnitIcons[i], fmt::format("queued_unit_icon_{}", i));
    }
}

void HUDUpdater::updateFactoryUnitCreations(uint32_t entity)
{
    m_creationInProgressGroup->setVisible(false);
    m_creationQueueGroup->setVisible(false);

    if (m_gameState->hasComponent<CompUnitFactory>(entity))
    {
        auto& factory = m_gameState->getComponent<CompUnitFactory>(entity);

        if (factory.productionQueue.empty() == false && factory.currentUnitProgress < 100)
        {
            auto displayName = m_typeRegistry->getHUDDisplayName(factory.productionQueue[0]);
            auto unitIcon = m_typeRegistry->getHUDIcon(factory.productionQueue[0]);
            m_progressTextLabel->setText(
                std::format("Creating - {}%", (int) factory.currentUnitProgress));

            m_progressItemNameLabel->setText(displayName);
            m_unitInProgressIcon->setBackgroundImage(unitIcon);

            auto& graphic = m_progressBarLabel->getBackgroundImage();
            graphic.variation = factory.currentUnitProgress;
            m_progressBarLabel->setBackgroundImage(graphic);

            m_creationInProgressGroup->setVisible(true);
        }

        for (int i = 1; i < Constants::ABSOLUTE_MAX_UNIT_QUEUE_SIZE; ++i)
        {
            if (i < factory.productionQueue.size())
            {
                auto unitIcon = m_typeRegistry->getHUDIcon(factory.productionQueue[i]);
                m_queuedUnitIcons[i - 1]->setBackgroundImage(unitIcon);
                m_queuedUnitIcons[i - 1]->setVisible(true);
                m_creationQueueGroup->setVisible(true);
            }
            else
            {
                m_queuedUnitIcons[i - 1]->setVisible(false);
            }
        }
    }
}

void HUDUpdater::updateBuildingConstruction(CompBuilding& building, CompEntityInfo& info)
{
    auto displayName = m_typeRegistry->getHUDDisplayName(info.entityType);

    m_progressTextLabel->setText(std::format("Building - {}%", building.constructionProgress));
    m_progressItemNameLabel->setText(displayName);
    auto& graphic = m_progressBarLabel->getBackgroundImage();
    graphic.variation = building.constructionProgress;
    m_progressBarLabel->setBackgroundImage(graphic);

    m_creationInProgressGroup->setVisible(true);
}
