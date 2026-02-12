#include "PlayerController.h"

#include "EntityFactory.h"
#include "Player.h"
#include "PlayerFactory.h"
#include "ServiceRegistry.h"
#include "StateManager.h"
#include "commands/CmdAttack.h"
#include "commands/CmdBuild.h"
#include "commands/CmdGarrison.h"
#include "commands/CmdGatherResource.h"
#include "commands/CmdMove.h"
#include "components/CompBuilder.h"
#include "components/CompBuilding.h"
#include "components/CompEntityInfo.h"
#include "components/CompGarrison.h"
#include "components/CompGraphics.h"
#include "components/CompPlayer.h"
#include "components/CompResource.h"
#include "components/CompTransform.h"
#include "components/CompUnit.h"
#include "logging/Logger.h"
#include "utils/Utils.h"

#include <SDL3/SDL_scancode.h>

using namespace core;

PlayerController::PlayerController()
{
    registerCallback(Event::Type::KEY_UP, this, &PlayerController::onKeyUp);
    registerCallback(Event::Type::MOUSE_BTN_UP, this, &PlayerController::onMouseButtonUp);
    registerCallback(Event::Type::MOUSE_MOVE, this, &PlayerController::onMouseMove);
    registerCallback(Event::Type::BUILDING_CREATED, this, &PlayerController::onBuildingApproved);
    registerCallback(Event::Type::MOUSE_BTN_DOWN, this, &PlayerController::onMouseButtonDown);
    registerCallback(Event::Type::ENTITY_SELECTION, this, &PlayerController::onUnitSelection);
}

void PlayerController::setPlayer(Ref<Player> player)
{
    m_player = player;
}

Ref<Player> PlayerController::getPlayer() const
{
    return m_player;
}

void PlayerController::resolveAction(const Vec2& screenPos)
{
    auto result = m_stateMan->whatIsAt(screenPos);
    auto target = result.entity;
    auto layer = result.layer;
    bool gatherable = m_stateMan->hasComponent<CompResource>(target);
    bool construction = m_stateMan->hasComponent<CompBuilding>(target);
    auto targetOwner = m_stateMan->tryGetComponent<CompPlayer>(target);
    bool isGarrisonInProgress = m_garrisonOperationInProgress;
    bool canAttack = targetOwner and (not m_player->isSame(targetOwner->player)) and
                     (not m_player->isAlly(targetOwner->player));

    for (auto entity : m_currentEntitySelection.selection.selectedEntities)
    {
        if (target == entt::null)
        {
            // Nothing at destination, probably request to move
            // TODO: Ensure target is within the map
            // TODO: Ensure we have control over this unit

            if (m_stateMan->hasComponent<CompUnit>(entity))
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
            // TODO: Check selected entities are capability to gather, if it isn't doable, fall back
            // to move
            auto cmd = ObjectPool<CmdGatherResource>::acquire();
            cmd->target = target;

            publishEvent(Event::Type::COMMAND_REQUEST, CommandRequestData{cmd, entity});
        }
        else if (isGarrisonInProgress and construction and m_player->isOwned(target))
        {
            // TODO: Original game allows to use both right and left clicks for garrison. Here we
            // use only the right click
            tryCompleteGarrison(entity, target);
        }
        else if (construction and m_player->isOwned(target))
        {
            auto cmd = ObjectPool<CmdBuild>::acquire();
            cmd->target = target;

            publishEvent(Event::Type::COMMAND_REQUEST, CommandRequestData{cmd, entity});
        }
        else if (canAttack)
        {
            auto cmd = ObjectPool<CmdAttack>::acquire();
            cmd->target = target;

            publishEvent(Event::Type::COMMAND_REQUEST, CommandRequestData{cmd, entity});
        }
    }
}

void PlayerController::createUnit(uint32_t entityType, const EntitySelection& selectedBuildings)
{
    spdlog::debug("Request to create unit type {}", entityType);

    for (auto building : selectedBuildings.selectedEntities)
    {
        UnitQueueData data{.player = m_player, .entityType = entityType, .building = building};
        publishEvent(Event::Type::UNIT_QUEUE_REQUEST, data);
    }
}

//=================================================================================================
//  Event callbacks
//=================================================================================================

void PlayerController::onKeyUp(const Event& e)
{
    ScopedDiagnosticContext scoped("P", m_player->getId());

    SDL_Scancode scancode = static_cast<SDL_Scancode>(e.getData<KeyboardData>().keyCode);

#ifdef DEVELOPMENT
    // Switching between first two players for testing purposes until we have
    // either network or AI players.
    //
    if (scancode == SDL_SCANCODE_1)
    {
        spdlog::info("Switching to player 1");
        auto facatory = ServiceRegistry::getInstance().getService<PlayerFactory>();
        setPlayer(facatory->getPlayer(1));
    }
    else if (scancode == SDL_SCANCODE_2)
    {
        spdlog::info("Switching to player 2");
        auto facatory = ServiceRegistry::getInstance().getService<PlayerFactory>();
        setPlayer(facatory->getPlayer(2));
    }
    else
#endif
    {
        if (scancode == SDL_SCANCODE_ESCAPE)
        {
            concludeAllBuildingPlacements();
        }
        else if (scancode == SDL_SCANCODE_LCTRL or scancode == SDL_SCANCODE_RCTRL)
        {
            rotateCurrentPlacement();
        }
        else
        {
            auto resolver = ServiceRegistry::getInstance().getService<ShortcutResolver>();
            auto action = resolver->resolve(scancode, m_currentEntitySelection);

            if (action.type == ShortcutResolver::Action::Type::CREATE_BUILDING)
            {
                startBuildingPlacement(action.entityType);
            }
            else if (action.type == ShortcutResolver::Action::Type::CREATE_UNIT)
            {
                createUnit(action.entityType, m_currentEntitySelection.selection);
            }
            else if (action.type == ShortcutResolver::Action::Type::GARRISON)
            {
                initiateGarrison();
            }
            else if (action.type == ShortcutResolver::Action::Type::UNGARRISON)
            {
                initiateUngarrison();
            }
        }
    }
}

void PlayerController::onMouseButtonUp(const Event& e)
{
    ScopedDiagnosticContext scoped("P", m_player->getId());

    auto& clickData = e.getData<MouseClickData>();
    auto& mousePos = clickData.screenPosition;

    if (clickData.button == MouseClickData::Button::LEFT)
    {
        // Left click priorities are;
        // 1) Finalizing ongoing building placement
        // 2) Entity selection by clicking
        //
        if (m_currentBuildingPlacements.empty() == false)
        {
            for (auto it = m_currentBuildingPlacements.begin();
                 it != m_currentBuildingPlacements.end();)
            {
                auto placement = it++;
                auto& building = m_stateMan->getComponent<CompBuilding>(placement->second.entity);

                if (building.validPlacement)
                    confirmBuildingPlacement(placement->second);
            }
            concludeAllBuildingPlacements();
        }
        else
        {
            selectEntities(mousePos);
        }
    }
    else if (clickData.button == MouseClickData::Button::RIGHT)
    {
        resolveAction(mousePos);
    }
    concludeGarrison();
}

void PlayerController::onMouseButtonDown(const Event& e)
{
    ScopedDiagnosticContext scoped("P", m_player->getId());

    auto& clickData = e.getData<MouseClickData>();

    if (clickData.button == MouseClickData::Button::LEFT)
    {
        if (m_currentBuildingPlacements.empty() == false)
        {
            if (m_seriesConstructionAllowed)
            {
                auto& screenPos = e.getData<MouseClickData>().screenPosition;
                auto feet = m_coordinates->screenUnitsToFeet(screenPos);

                m_seriesConstructionInitiated = true;
                m_firstBuildingPos = feet.toTile();

                auto& firstBuilding = m_currentBuildingPlacements.begin()->second;
                spdlog::debug(
                    "Series of constructions are initiated for building type {} at tile {}",
                    firstBuilding.entityType, firstBuilding.pos.toTile().toString());
            }
        }
        else
        {
            m_isSelectionBoxInProgress = true;
            m_selectionStartPosScreenUnits = clickData.screenPosition;
        }
    }
}

void PlayerController::onMouseMove(const Event& e)
{
    ScopedDiagnosticContext scoped("P", m_player->getId());

    m_currentMouseScreenPos = e.getData<MouseMoveData>().screenPos;
    auto feet = m_coordinates->screenUnitsToFeet(m_currentMouseScreenPos);

    // All the remaining tasks are tile based, therefore, if the mouse hasn't moved
    // out of the previous tile, then no need to continue
    if (feet.toTile() == m_lastMouseTile)
        return;
    m_lastMouseTile = feet.toTile();

    if (m_seriesConstructionInitiated && m_currentBuildingPlacements.empty() == false)
    {
        // Create more building placements to join from the start tile to current instead of moving
        // the existing building placement.

        auto end = feet.toTile();
        auto& start = m_firstBuildingPos;
        std::list<TilePosWithOrientation> newConnectedBuildingPositions;
        auto buildingType = m_currentBuildingPlacements.begin()->second.entityType;

        Utils::calculateConnectedBuildingsPath(start, end, newConnectedBuildingPositions);
        removeAllExistingBuildingPlacements();
        createConnectedBuildingPlacements(newConnectedBuildingPositions, buildingType);
    }
    else
    {
        // Typical usage, just move the existing building placement
        if (m_currentBuildingPlacements.size() == 1)
        {
            auto placement = m_currentBuildingPlacements.begin();
            validateAndSnapBuildingToTile(placement->second, feet);
        }
    }
}

// Building was approved by the BuildingManager, time to assign the currently
// selected villagers (TODO: validate) to build it.
//
void PlayerController::onBuildingApproved(const Event& e)
{
    ScopedDiagnosticContext scoped("P", m_player->getId());

    auto& data = e.getData<BuildingPlacementData>();
    if (data.player == m_player)
    {
        for (auto unit : m_currentEntitySelection.selection.selectedEntities)
        {
            if (m_stateMan->hasComponent<CompBuilder>(unit))
            {
                // Send build instruction
                auto cmd = ObjectPool<CmdBuild>::acquire();
                cmd->target = data.entity;

                publishEvent(Event::Type::COMMAND_REQUEST, CommandRequestData{cmd, unit});
            }
        }
    }
}

//=================================================================================================
//  Building placement related
//=================================================================================================

void PlayerController::startBuildingPlacement(uint32_t buildingType)
{
    auto worldPos = m_coordinates->screenUnitsToFeet(m_currentMouseScreenPos);

    auto placement =
        createBuildingPlacement(buildingType, worldPos, BuildingOrientation::NO_ORIENTATION);
    publishEvent(Event::Type::BUILDING_PLACEMENT_STARTED, placement);
    // Just by pressing shortcut or button in UI to start creating a building doesn't mean
    // player intend to create series of same buildings. Series starts only when player press
    // down mouse left button.
    //
    m_seriesConstructionInitiated = false;
}

/*
 *   Approach:
 *       1. Create new or recycle building entity
 *       2. Validate and snap building to the tile
 *       3. Create building placement data and track
 */
BuildingPlacementData PlayerController::createBuildingPlacement(
    uint32_t buildingType, const Feet& pos, const BuildingOrientation& orientation)
{
    uint32_t entity = getOrCreateBuildingEntity(buildingType);

    auto [transform, playerComp, building, info] =
        m_stateMan->getComponents<CompTransform, CompPlayer, CompBuilding, CompEntityInfo>(entity);

    info.isDestroyed = false;
    playerComp.player = m_player;
    building.orientation = orientation;

    // No specific orientation was provided, have to default to the value configured
    // in the building.
    if (orientation == BuildingOrientation::NO_ORIENTATION)
    {
        building.orientation = building.defaultOrientation;
    }

    spdlog::debug("Creating building placement with orientation {} at {}",
                  buildingOrientationToString(building.orientation), pos.toTile().toString());

    building.constructionProgress = 100; // Treat as a complete building
    StateManager::markDirty(entity);

    // building position (hence landarea) will be updated later based on the result of following
    // query. but to check whether building can be placed there, we need a building with updated
    // landarea, hence creating a temporary building.
    CompBuilding tempBuilding = building;
    tempBuilding.updateLandArea(pos);
    bool outOfMap = false;
    building.validPlacement = m_stateMan->canPlaceBuildingAt(tempBuilding, outOfMap);

    if (!outOfMap)
    {
        transform.position = building.getSnappedBuildingCenter(pos);
        building.updateLandArea(transform.position);
    }
    else
    {
        // TODO: Where to display?
    }

#ifndef NDEBUG
    auto& graphics = m_stateMan->getComponent<CompGraphics>(entity);
    graphics.debugOverlays.clear();

    for (auto& tile : building.landArea.tiles)
    {
        DebugOverlay filled;
        filled.type = DebugOverlay::Type::FILLED_RHOMBUS;
        filled.color = Color::RED;
        filled.color.a = 100;
        const auto centerInFeet = tile.centerInFeet();
        const int half = Constants::FEET_PER_TILE / 2;
        filled.rhombusCorners[0] = centerInFeet + Feet(-half, half);
        filled.rhombusCorners[1] = centerInFeet + Feet(-half, -half);
        filled.rhombusCorners[2] = centerInFeet + Feet(half, -half);
        filled.rhombusCorners[3] = centerInFeet + Feet(half, half);
        graphics.debugOverlays.push_back(filled);
    }

    DebugOverlay anchor;
    anchor.type = DebugOverlay::Type::FILLED_CIRCLE;
    anchor.color = Color::BLACK;
    anchor.circlePixelRadius = 10;
    anchor.absolutePosition = transform.position;
    graphics.debugOverlays.push_back(anchor);

#endif

    BuildingPlacementData data;
    data.player = m_player;
    data.entityType = buildingType;
    data.pos = pos;
    data.entity = entity;
    data.orientation = building.orientation;
    m_seriesConstructionAllowed = building.connectedConstructionsAllowed;

    m_currentBuildingPlacements[data.placementId] = data;

    return data;
}

/*
 *   Creating an entity is relatively expensive, therefore, this will try recycle
 *   an entity from pool.
 */
uint32_t PlayerController::getOrCreateBuildingEntity(uint32_t buildingType)
{
    uint32_t entity = entt::null;

    if (m_entityByTypeRecyclePool.contains(buildingType) and
        m_entityByTypeRecyclePool.at(buildingType).empty() == false)
    {
        entity = std::move(m_entityByTypeRecyclePool[buildingType].front());
        m_entityByTypeRecyclePool[buildingType].pop_front();
    }
    else
    {
        auto factory = ServiceRegistry::getInstance().getService<EntityFactory>();
        entity = factory->createEntity(buildingType);
    }
    return entity;
}

void PlayerController::removeAllExistingBuildingPlacements()
{
    for (const auto& it : m_currentBuildingPlacements)
    {
        auto entity = it.second.entity;
        auto& info = m_stateMan->getComponent<CompEntityInfo>(entity);

        info.isDestroyed = true;
        StateManager::markDirty(entity);
        m_entityByTypeRecyclePool[info.entityType].push_back(entity); // Recycle the entity
    }
    m_currentBuildingPlacements.clear();
}

void PlayerController::createConnectedBuildingPlacements(
    const std::list<TilePosWithOrientation>& connectedBuildings, uint32_t buildingType)
{
    for (auto& connectedBuilding : connectedBuildings)
    {
        auto placement = createBuildingPlacement(buildingType, connectedBuilding.pos.toFeet(),
                                                 connectedBuilding.orientation);
    }
}

void PlayerController::validateAndSnapBuildingToTile(BuildingPlacementData& placement,
                                                     const Feet& pos)
{
    auto [transform, building] =
        m_stateMan->getComponents<CompTransform, CompBuilding>(placement.entity);

    bool outOfMap = false;
    building.validPlacement = m_stateMan->canPlaceBuildingAt(building, outOfMap);

    if (!outOfMap)
    {
        transform.position = building.getSnappedBuildingCenter(pos); // Snap
        building.updateLandArea(transform.position);
    }
    placement.pos = transform.position;
    StateManager::markDirty(placement.entity);

#ifndef NDEBUG
    auto& graphics = m_stateMan->getComponent<CompGraphics>(placement.entity);
    graphics.debugOverlays.clear();

    for (auto& tile : building.landArea.tiles)
    {
        DebugOverlay filled;
        filled.type = DebugOverlay::Type::FILLED_RHOMBUS;
        filled.color = Color::RED;
        filled.color.a = 100;
        const auto centerInFeet = tile.centerInFeet();
        const int half = Constants::FEET_PER_TILE / 2;
        filled.rhombusCorners[0] = centerInFeet + Feet(-half, half);
        filled.rhombusCorners[1] = centerInFeet + Feet(-half, -half);
        filled.rhombusCorners[2] = centerInFeet + Feet(half, -half);
        filled.rhombusCorners[3] = centerInFeet + Feet(half, half);
        graphics.debugOverlays.push_back(filled);
    }

    DebugOverlay anchor;
    anchor.type = DebugOverlay::Type::FILLED_CIRCLE;
    anchor.color = Color::BLACK;
    anchor.circlePixelRadius = 10;
    anchor.absolutePosition = transform.position;
    graphics.debugOverlays.push_back(anchor);

#endif
}

void PlayerController::confirmBuildingPlacement(const BuildingPlacementData& placement) const
{
    auto [building, transform, player, info] =
        m_stateMan->getComponents<CompBuilding, CompTransform, CompPlayer, CompEntityInfo>(
            placement.entity);

    // Player wants to place the building, but it is BuildingManager's responsibility
    // to do it. Therefore, requesting building.
    //
    BuildingPlacementData data = placement;
    data.entityType = info.entityType;
    data.pos = transform.position;
    publishEvent(Event::Type::BUILDING_REQUESTED, data);
}

// Careful when calling this inside a m_currentBuildingPlacements loop
void PlayerController::concludeBuildingPlacement(uint32_t placementId)
{
    if (m_currentBuildingPlacements.contains(placementId))
    {
        auto& placement = m_currentBuildingPlacements[placementId];
        auto& info = m_stateMan->getComponent<CompEntityInfo>(placement.entity);

        info.isDestroyed = true;
        StateManager::markDirty(placement.entity);

        publishEvent(Event::Type::BUILDING_PLACEMENT_ENDED, placement);

        m_currentBuildingPlacements.erase(placementId);
    }
}

void PlayerController::concludeAllBuildingPlacements()
{
    for (auto it = m_currentBuildingPlacements.begin(); it != m_currentBuildingPlacements.end();)
    {
        auto current = it++;
        concludeBuildingPlacement(current->second.placementId);
    }
}

// =================================================================================================
//  Garrison related
// =================================================================================================

void PlayerController::initiateGarrison()
{
    spdlog::debug("Garrison initiated");
    m_garrisonOperationInProgress = true;
    publishEvent(Event::Type::GARRISON_REQUEST,
                 GarrisonData{.player = m_player, .inprogress = true});
}

void PlayerController::initiateUngarrison()
{
    if (m_currentEntitySelection.type == EntitySelectionData::Type::BUILDING &&
        m_currentEntitySelection.selection.selectedEntities.empty() == false)
    {
        publishEvent(
            Event::Type::UNGARRISON_REQUEST,
            UngarrisonData{m_player, m_currentEntitySelection.selection.selectedEntities[0]});
    }
}

void PlayerController::concludeGarrison()
{
    if (m_garrisonOperationInProgress)
    {
        m_garrisonOperationInProgress = false;
        publishEvent(Event::Type::GARRISON_REQUEST,
                     GarrisonData{.player = m_player, .inprogress = false});
    }
}

void PlayerController::tryCompleteGarrison(uint32_t unitId, uint32_t targetBuildingEntityId)
{
    auto garrison = m_stateMan->tryGetComponent<CompGarrison>(targetBuildingEntityId);
    if (garrison != nullptr && m_currentEntitySelection.type == EntitySelectionData::Type::UNIT)
    {
        auto& unitComp = m_stateMan->getComponent<CompUnit>(unitId);
        if (garrison->unitTypes.value().contains(unitComp.type))
        {
            auto cmd = ObjectPool<CmdGarrison>::acquire();
            cmd->target = targetBuildingEntityId;

            publishEvent(Event::Type::COMMAND_REQUEST, CommandRequestData{cmd, unitId});
        }
    }
}

// =================================================================================================
//  Selection related
// =================================================================================================

void PlayerController::selectEntities(const Vec2& screenPos)
{
    // Invalidate in-progress selection if mouse hasn't moved much. Essentially, avoiding
    // tiny selection boxes. Which will result in treating this as a click-to-select.
    //
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
    //
    if (m_isSelectionBoxInProgress)
    {
        // TODO: we want on-the-fly selection instead of mouse button up
        completeSelectionBox(m_selectionStartPosScreenUnits, screenPos);
        m_isSelectionBoxInProgress = false;
    }
    else
    {
        handleClickToSelect(screenPos);
    }
}

/*
 *   This function try to add given list of entities to the selection. It may fail to add
 *   all requested entities since it ensures selection is homogeneous. eg: units and
 *   buildings cannot be in the same selection.
 */
void PlayerController::selectHomogeneousEntities(const std::vector<uint32_t>& selectedEntities)
{
    bool containsUnits = false;
    bool containsBuildings = false;
    bool containsNaturalResources = false;
    EntitySelectionData::Type selectionType = EntitySelectionData::Type::UNIT;

    for (auto entity : selectedEntities)
    {
        if (m_stateMan->hasComponent<CompUnit>(entity)) [[likely]]
        {
            containsUnits = true;
            selectionType = EntitySelectionData::Type::UNIT;
            break;
        }
        else if (m_stateMan->hasComponent<CompBuilding>(entity))
        {
            containsBuildings = true;
            selectionType = EntitySelectionData::Type::BUILDING;
            break;
        }
        else if (m_stateMan->hasComponent<CompResource>(entity))
        {
            containsNaturalResources = true;
            selectionType = EntitySelectionData::Type::NATURAL_RESOURCE;
            break;
        }
    }

    EntitySelection selection;

    for (auto entity : selectedEntities)
    {
        bool isValid = false;
        if (containsUnits) [[likely]]
        {
            isValid = m_stateMan->hasComponent<CompUnit>(entity);
        }
        else if (containsBuildings)
        {
            isValid = m_stateMan->hasComponent<CompBuilding>(entity);
        }
        else if (containsNaturalResources)
        {
            isValid = m_stateMan->hasComponent<CompResource>(entity);
        }
        else [[unlikely]]
        {
            spdlog::error("Failed to select entity {}, entity is not selectible", entity);
            continue;
        }

        if (isValid)
        {
            auto& select = m_stateMan->getComponent<CompSelectible>(entity);

            selection.selectedEntities.push_back(entity);
            select.isSelected = true;
            StateManager::markDirty(entity);
        }
    }

    if (!containsUnits && !containsBuildings && !containsNaturalResources)
    {
        return;
    }
    publishEvent(Event::Type::ENTITY_SELECTION,
                 EntitySelectionData{.type = selectionType, .selection = selection});
}

void PlayerController::handleClickToSelect(const Vec2& screenPos)
{
    auto result = m_stateMan->whatIsAt(screenPos);

    // FIXME: Doesn't work for units
    if (result.entity != entt::null)
    {
        selectHomogeneousEntities({result.entity});
    }
    else
    {
        publishEvent(Event::Type::ENTITY_SELECTION, EntitySelectionData());
    }
}

void PlayerController::completeSelectionBox(const Vec2& startScreenPos, const Vec2& endScreenPos)
{
    // FIXME: Doing multiple drag selection seems to break the selection logic
    std::vector<uint32_t> overlappingEntities;

    getAllOverlappingEntities(startScreenPos, endScreenPos, overlappingEntities);
    selectHomogeneousEntities(overlappingEntities);
}

/*
    PlayerManager gets unit selection changes as events instead of relying on mouse events to
    decouple selection and other selection-based activities (such as movements). Additionally
    it allows integ tests to control unit selection easily using events.
 */
void PlayerController::onUnitSelection(const Event& e)
{
    DiagnosticContext::getInstance().put("P", m_player->getId());

    updateSelection(e.getData<EntitySelectionData>());
}

void PlayerController::updateSelection(const EntitySelectionData& newSelection)
{
    // Clear any existing selections
    for (auto& entity : m_currentEntitySelection.selection.selectedEntities)
    {
        auto& select = m_stateMan->getComponent<CompSelectible>(entity);

        select.isSelected = false;
        StateManager::markDirty(entity);
    }
    m_currentEntitySelection = newSelection;
}

void PlayerController::getAllOverlappingEntities(const Vec2& startScreenPos,
                                                 const Vec2& endScreenPos,
                                                 std::vector<uint32_t>& entitiesToAddToSelection)
{
    int selectionLeft = std::min(startScreenPos.x, endScreenPos.x);
    int selectionRight = std::max(startScreenPos.x, endScreenPos.x);
    int selectionTop = std::min(startScreenPos.y, endScreenPos.y);
    int selectionBottom = std::max(startScreenPos.y, endScreenPos.y);

    auto player = getPlayer();

    // Following consider only units since selection box cannot be used for non-unit entities
    // such as buildings and trees
    // TODO: Optimize this by using tilemap
    m_stateMan->getEntities<CompUnit, CompTransform, CompPlayer>().each(
        [this, selectionLeft, selectionRight, selectionTop, selectionBottom, &player,
         &entitiesToAddToSelection](uint32_t entity, CompUnit& unit, CompTransform& transform,
                                    CompPlayer& playerComp)
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
}

void PlayerController::rotateCurrentPlacement()
{
    if (m_currentBuildingPlacements.size() == 1)
    {
        auto& placement = m_currentBuildingPlacements.begin()->second;
        if (placement.orientation == BuildingOrientation::DIAGONAL_BACKWARD)
        {
            changePlacementOrientation(placement, BuildingOrientation::HORIZONTAL);
        }
        else if (placement.orientation == BuildingOrientation::HORIZONTAL)
        {
            changePlacementOrientation(placement, BuildingOrientation::DIAGONAL_FORWARD);
        }
        else if (placement.orientation == BuildingOrientation::DIAGONAL_FORWARD)
        {
            changePlacementOrientation(placement, BuildingOrientation::VERTICAL);
        }
        else if (placement.orientation == BuildingOrientation::VERTICAL)
        {
            changePlacementOrientation(placement, BuildingOrientation::DIAGONAL_BACKWARD);
        }
    }
}

void PlayerController::changePlacementOrientation(BuildingPlacementData& placement,
                                                  BuildingOrientation orientation)
{
    placement.orientation = orientation;
    auto& building = m_stateMan->getComponent<CompBuilding>(placement.entity);
    building.orientation = placement.orientation;
    auto feet = m_coordinates->screenUnitsToFeet(m_currentMouseScreenPos);
    validateAndSnapBuildingToTile(placement, feet);
    StateManager::markDirty(placement.entity);
}
