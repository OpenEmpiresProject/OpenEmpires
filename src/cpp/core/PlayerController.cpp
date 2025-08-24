#include "PlayerController.h"

#include "GameState.h"
#include "Player.h"
#include "PlayerFactory.h"
#include "ServiceRegistry.h"
#include "commands/CmdBuild.h"
#include "commands/CmdGatherResource.h"
#include "commands/CmdMove.h"
#include "components/CompBuilding.h"
#include "components/CompDirty.h"
#include "components/CompEntityInfo.h"
#include "components/CompPlayer.h"
#include "components/CompResource.h"
#include "components/CompTransform.h"
#include "components/CompUnit.h"
#include "utils/Logger.h"

#include <SDL3/SDL_scancode.h>

using namespace core;

PlayerController::PlayerController()
    : m_coordinates(ServiceRegistry::getInstance().getService<Coordinates>())
{
    m_gameState = ServiceRegistry::getInstance().getService<GameState>();

    registerCallback(Event::Type::KEY_UP, this, &PlayerController::onKeyUp);
    registerCallback(Event::Type::MOUSE_BTN_UP, this, &PlayerController::onMouseButtonUp);
    registerCallback(Event::Type::MOUSE_BTN_DOWN, this, &PlayerController::onMouseButtonDown);
    registerCallback(Event::Type::ENTITY_SELECTION, this, &PlayerController::onUnitSelection);
    registerCallback(Event::Type::BUILDING_PLACEMENT_STARTED, this,
                     &PlayerController::onBuildingPlacementStarted);
    registerCallback(Event::Type::BUILDING_PLACEMENT_FINISHED, this,
                     &PlayerController::onBuildingPlacementFinished);
}

PlayerController::~PlayerController()
{
    // destructor
}

void PlayerController::onKeyUp(const Event& e)
{
    SDL_Scancode scancode = static_cast<SDL_Scancode>(e.getData<KeyboardData>().keyCode);

    if (scancode == SDL_SCANCODE_1)
    {
        spdlog::info("Switching to player 1");
        auto facatory = ServiceRegistry::getInstance().getService<PlayerFactory>();
        m_player = facatory->getPlayer(1);
    }
    else if (scancode == SDL_SCANCODE_2)
    {
        spdlog::info("Switching to player 2");
        auto facatory = ServiceRegistry::getInstance().getService<PlayerFactory>();
        m_player = facatory->getPlayer(2);
    }
}

void PlayerController::setPlayer(Ref<Player> player)
{
    m_player = player;
}

void PlayerController::onMouseButtonUp(const Event& e)
{
    auto& clickData = e.getData<MouseClickData>();
    auto mousePos = clickData.screenPosition;

    if (clickData.button == MouseClickData::Button::LEFT)
    {
        handleEntitySelection(mousePos);
    }
    else if (clickData.button == MouseClickData::Button::RIGHT)
    {
        resolveAction(mousePos);
    }
}

void PlayerController::onMouseButtonDown(const Event& e)
{
    auto& clickData = e.getData<MouseClickData>();

    if (clickData.button == MouseClickData::Button::LEFT)
    {
        m_isSelectionBoxInProgress = true;
        m_selectionStartPosScreenUnits = clickData.screenPosition;
    }
}

void PlayerController::handleEntitySelection(const Vec2& screenPos)
{
    // BuildingManager will take care mouse clicks if there is a ongoing building
    // placement.
    if (m_buildingPlacementInProgress)
        return;

    // Invalidate in-progress selection if mouse hasn't moved much. Essentially, avoiding
    // tiny selection boxes.
    if (m_isSelectionBoxInProgress)
    {
        if ((screenPos - m_selectionStartPosScreenUnits).lengthSquared() <
            Constants::MIN_SELECTION_BOX_MOUSE_MOVEMENT)
        {
            m_isSelectionBoxInProgress = false;
        }
    }

    // Selection priorities are;
    // 1. Complete selection box
    // 2. Individual selection click

    if (m_isSelectionBoxInProgress)
    {
        // TODO: we want on the fly selection instead of mouse button up
        completeSelectionBox(m_selectionStartPosScreenUnits, screenPos);
        m_isSelectionBoxInProgress = false;
    }
    else
    {
        handleClickToSelect(screenPos);
    }
}

void PlayerController::resolveAction(const Vec2& screenPos)
{
    auto result = m_gameState->whatIsAt(screenPos);
    auto target = result.entity;
    auto layer = result.layer;
    auto m_gameState = ServiceRegistry::getInstance().getService<GameState>();
    bool gatherable = m_gameState->hasComponent<CompResource>(target);
    bool construction = m_gameState->hasComponent<CompBuilding>(target);

    for (auto entity : m_currentUnitSelection.selectedEntities)
    {
        if (target == entt::null)
        {
            // Nothing at destination, probably request to move
            // TODO: Ensure target is within the map
            // TODO: Ensure we have control over this unit

            if (m_gameState->hasComponent<CompUnit>(entity))
            {
                auto worldPos = m_coordinates->screenUnitsToFeet(screenPos);
                auto cmd = ObjectPool<CmdMove>::acquire();
                cmd->targetPos = worldPos;

                publishEvent(Event::Type::COMMAND_REQUEST, CommandRequestData{cmd, entity});
            }
        }
        else if (gatherable)
        {
            // It is a request to gather the resource at target
            // TODO: Check selected entity's capability to gather, if it isn't doable, fall back to
            // move
            auto cmd = ObjectPool<CmdGatherResource>::acquire();
            cmd->target = target;

            publishEvent(Event::Type::COMMAND_REQUEST, CommandRequestData{cmd, entity});
        }
        else if (construction)
        {
            auto cmd = ObjectPool<CmdBuild>::acquire();
            cmd->target = target;

            publishEvent(Event::Type::COMMAND_REQUEST, CommandRequestData{cmd, entity});
        }
    }
}

/*
 *   This function try to add given list of entities to the selection. It may fail to add
 *   since it ensures selection is homogeneous. eg: units and buildings cannot be in the
 *   same selection.
 */
void PlayerController::trySelectingEntities(const std::vector<uint32_t>& selectedEntities)
{
    EntitySelection selection;
    auto m_gameState = ServiceRegistry::getInstance().getService<GameState>();

    bool containsUnits = false;
    for (auto entity : selectedEntities)
    {
        if (m_gameState->hasComponent<CompUnit>(entity)) [[likely]]
        {
            containsUnits = true;
            break;
        }
    }

    bool containsBuildings = false;
    if (containsUnits == false)
    {
        for (auto entity : selectedEntities)
        {
            if (m_gameState->hasComponent<CompBuilding>(entity)) [[likely]]
            {
                containsBuildings = true;
                break;
            }
        }
    }

    bool containsNaturalResources = false;
    if (containsUnits == false && containsBuildings == false)
    {
        for (auto entity : selectedEntities)
        {
            if (m_gameState->hasComponent<CompResource>(entity)) [[likely]]
            {
                containsNaturalResources = true;
                break;
            }
        }
    }

    for (auto entity : selectedEntities)
    {
        bool isValid = false;
        if (containsUnits) [[likely]]
        {
            isValid = m_gameState->hasComponent<CompUnit>(entity);
        }
        else if (containsBuildings)
        {
            isValid = m_gameState->hasComponent<CompBuilding>(entity);
        }
        else if (containsNaturalResources)
        {
            isValid = m_gameState->hasComponent<CompResource>(entity);
        }
        else [[unlikely]]
        {
            spdlog::error("Failed to select entity {}, entity is not selectible", entity);
            continue;
        }

        if (isValid)
        {
            auto [dirty, select] = m_gameState->getComponents<CompDirty, CompSelectible>(entity);

            selection.selectedEntities.push_back(entity);
            select.isSelected = true;
            dirty.markDirty(entity);
        }
    }

    EntitySelectionData::Type selectionType = EntitySelectionData::Type::UNIT;
    if (containsUnits)
    {
        selectionType = EntitySelectionData::Type::UNIT;
    }
    else if (containsBuildings)
    {
        selectionType = EntitySelectionData::Type::BUILDING;
    }
    else if (containsNaturalResources)
    {
        selectionType = EntitySelectionData::Type::NATURAL_RESOURCE;
    }
    else
    {
        return;
    }
    publishEvent(Event::Type::ENTITY_SELECTION,
                 EntitySelectionData{.type = selectionType, .selection = selection});
}

void PlayerController::handleClickToSelect(const Vec2& screenPos)
{
    auto result = m_gameState->whatIsAt(screenPos);

    // FIXME: Doesn't work for units
    if (result.entity != entt::null)
    {
        trySelectingEntities({result.entity});
    }
    else
    {
        publishEvent(Event::Type::ENTITY_SELECTION, EntitySelectionData());
    }
}

void PlayerController::completeSelectionBox(const Vec2& startScreenPos, const Vec2& endScreenPos)
{
    // FIXME: Doing multiple drag selection seems to break the selection logic

    int selectionLeft = std::min(startScreenPos.x, endScreenPos.x);
    int selectionRight = std::max(startScreenPos.x, endScreenPos.x);
    int selectionTop = std::min(startScreenPos.y, endScreenPos.y);
    int selectionBottom = std::max(startScreenPos.y, endScreenPos.y);

    auto player = getPlayer();

    std::vector<uint32_t> entitiesToAddToSelection;
    // Following consider only units since selection box cannot be used for non-unit entities
    // such as buildings and trees
    // TODO: Optimize this by using tilemap
    m_gameState->getEntities<CompUnit, CompTransform, CompDirty, CompPlayer>().each(
        [this, selectionLeft, selectionRight, selectionTop, selectionBottom, &player,
         &entitiesToAddToSelection](uint32_t entity, CompUnit& unit, CompTransform& transform,
                                    CompDirty& dirty, CompPlayer& playerComp)
        {
            // Cannot select other players' units
            if (playerComp.player != player)
                return;

            auto screenPos = m_coordinates->feetToScreenUnits(transform.position);
            int unitRight = screenPos.x + transform.selectionBoxWidth / 2;
            int unitBottom = screenPos.y;
            int unitLeft = screenPos.x - transform.selectionBoxWidth / 2;
            int unitTop = screenPos.y - transform.selectionBoxHeight;

            bool intersects = !(unitRight < selectionLeft || unitLeft > selectionRight ||
                                unitBottom < selectionTop || unitTop > selectionBottom);

            if (intersects)
            {
                spdlog::info("Unit {} is selected", entity);

                entitiesToAddToSelection.push_back(entity);
            }
        });

    trySelectingEntities(entitiesToAddToSelection);
}

void PlayerController::onBuildingPlacementStarted(const Event& e)
{
    m_buildingPlacementInProgress = true;
}

void PlayerController::onBuildingPlacementFinished(const Event& e)
{
    m_buildingPlacementInProgress = false;
}

/*
    PlayerManager gets unit selection changes as events instead of relying on mouse events to
    decouple selection and other selection-based activities (such as movements). Additionally
    it allows integ tests to control unit selection easily using events.
 */
void PlayerController::onUnitSelection(const Event& e)
{
    updateSelection(e.getData<EntitySelectionData>().selection);
}

void PlayerController::updateSelection(const EntitySelection& newSelection)
{
    auto m_gameState = ServiceRegistry::getInstance().getService<GameState>();
    // Clear any existing selections' addons
    for (auto& entity : m_currentUnitSelection.selectedEntities)
    {
        auto [dirty, select] = m_gameState->getComponents<CompDirty, CompSelectible>(entity);

        select.isSelected = false;
        dirty.markDirty(entity);
    }
    m_currentUnitSelection = newSelection;
}
