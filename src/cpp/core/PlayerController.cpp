#include "PlayerController.h"

#include "EntityFactory.h"
#include "Player.h"
#include "PlayerFactory.h"
#include "ServiceRegistry.h"
#include "StateManager.h"
#include "commands/CmdBuild.h"
#include "commands/CmdGarrison.h"
#include "commands/CmdGatherResource.h"
#include "commands/CmdMove.h"
#include "components/CompBuilder.h"
#include "components/CompBuilding.h"
#include "components/CompDirty.h"
#include "components/CompEntityInfo.h"
#include "components/CompGarrison.h"
#include "components/CompPlayer.h"
#include "components/CompResource.h"
#include "components/CompTransform.h"
#include "components/CompUnit.h"
#include "utils/Logger.h"

#include <SDL3/SDL_scancode.h>

using namespace core;

PlayerController::PlayerController()
{
    m_coordinates = ServiceRegistry::getInstance().getService<Coordinates>();
    m_stateMan = ServiceRegistry::getInstance().getService<StateManager>();

    registerCallback(Event::Type::KEY_UP, this, &PlayerController::onKeyUp);
    registerCallback(Event::Type::MOUSE_BTN_UP, this, &PlayerController::onMouseButtonUp);
    registerCallback(Event::Type::MOUSE_MOVE, this, &PlayerController::onMouseMove);
    registerCallback(Event::Type::BUILDING_APPROVED, this, &PlayerController::onBuildingApproved);
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
    auto m_stateMan = ServiceRegistry::getInstance().getService<StateManager>();
    bool gatherable = m_stateMan->hasComponent<CompResource>(target);
    bool construction = m_stateMan->hasComponent<CompBuilding>(target);
    bool isGarrisonInProgress = m_garrisonOperationInProgress;

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
        else if (isGarrisonInProgress && construction)
        {
            // TODO: Original game allows to use both right and left clicks for garrison. Here we
            // use only the right click
            tryCompleteGarrison(entity, target);
        }
        else if (construction)
        {
            auto cmd = ObjectPool<CmdBuild>::acquire();
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
    m_currentMouseScreenPos = e.getData<MouseMoveData>().screenPos;
    auto feet = m_coordinates->screenUnitsToFeet(m_currentMouseScreenPos);

    if (m_seriesConstructionInitiated && m_currentBuildingPlacements.empty() == false)
    {
        // Create more building placements to join from the start tile to current instead of moving
        // the existing building placement.

        auto end = feet.toTile();
        auto& start = m_firstBuildingPos;
        std::list<ConnectedBuildingPosition> newConnectedBuildingPositions;
        auto buildingType = m_currentBuildingPlacements.begin()->second.entityType;

        calculateConnectedBuildingsPath(start, end, newConnectedBuildingPositions);
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

    auto placement = createBuildingPlacement(buildingType, worldPos, BuildingOrientation::DEFAULT);
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

    auto [transform, playerComp, building, info, dirty] =
        m_stateMan
            ->getComponents<CompTransform, CompPlayer, CompBuilding, CompEntityInfo, CompDirty>(
                entity);

    bool outOfMap = false;
    building.validPlacement = m_stateMan->canPlaceBuildingAt(building, pos, outOfMap);

    if (!outOfMap)
    {
        transform.position = building.getTileSnappedPosition(pos);
    }
    else
    {
        // TODO: Where to display?
    }

    info.isDestroyed = false;
    playerComp.player = m_player;
    building.orientation = orientation;

    // Override default orientation with corner for connected buildings
    if (building.connectedConstructionsAllowed and orientation == BuildingOrientation::DEFAULT)
    {
        building.orientation = BuildingOrientation::CORNER;
    }
    building.constructionProgress = 100; // Treat as a complete building
    dirty.markDirty(entity);

    BuildingPlacementData data;
    data.player = m_player;
    data.entityType = buildingType;
    data.pos = pos;
    data.entity = entity;
    data.orientation = orientation;
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
        entity = factory->createEntity(buildingType, 0);
    }
    return entity;
}

void PlayerController::removeAllExistingBuildingPlacements()
{
    for (const auto& it : m_currentBuildingPlacements)
    {
        auto entity = it.second.entity;
        auto [info, dirty] = m_stateMan->getComponents<CompEntityInfo, CompDirty>(entity);

        info.isDestroyed = true;
        dirty.markDirty(entity);
        m_entityByTypeRecyclePool[info.entityType].push_back(entity); // Recycle the entity
    }
    m_currentBuildingPlacements.clear();
}

void PlayerController::createConnectedBuildingPlacements(
    const std::list<ConnectedBuildingPosition>& connectedBuildings, uint32_t buildingType)
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
    auto [transform, dirty, building] =
        m_stateMan->getComponents<CompTransform, CompDirty, CompBuilding>(placement.entity);

    bool outOfMap = false;
    building.validPlacement = m_stateMan->canPlaceBuildingAt(building, pos, outOfMap);

    if (!outOfMap)
    {
        transform.position = building.getTileSnappedPosition(pos);
    }
    placement.pos = transform.position;
    dirty.markDirty(placement.entity);
}

void PlayerController::confirmBuildingPlacement(const BuildingPlacementData& placement) const
{
    auto [building, transform, player, info, dirty] =
        m_stateMan
            ->getComponents<CompBuilding, CompTransform, CompPlayer, CompEntityInfo, CompDirty>(
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
        auto [info, dirty] = m_stateMan->getComponents<CompEntityInfo, CompDirty>(placement.entity);

        info.isDestroyed = true;
        dirty.markDirty(placement.entity);

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

/*
 *   Behavior:
 *       1. The path always consists of at most one 90° turn
 *       2. The corner orientation (L-shape) depends on drag direction:
 *           - If you drag more horizontally, the path goes horizontal first, then vertical.
 *           - If you drag more vertically, it goes vertical first, then horizontal.
 *       3. If the horizontal and vertical drags are same, then path is diagonal
 */
void PlayerController::calculateConnectedBuildingsPath(
    const Tile& start, const Tile& end, std::list<ConnectedBuildingPosition>& connectedBuildings)
{
    const int dx = std::abs(start.x - end.x);
    const int dy = std::abs(start.y - end.y);

    Tile corner = Tile::null;

    if (dx > dy)
    {
        corner = Tile(end.x, start.y);

        uint32_t x = std::min(start.x, end.x) + 1;
        const uint32_t maxX = std::max(start.x, end.x);

        for (; x < maxX; ++x)
        {
            Tile newPos(x, start.y);
            connectedBuildings.push_back(
                ConnectedBuildingPosition{newPos, BuildingOrientation::LEFT_ANGLED});
        }

        uint32_t y = std::min(start.y, end.y) + 1;
        const uint32_t maxY = std::max(start.y, end.y);

        for (; y < maxY; ++y)
        {
            Tile newPos(end.x, y);
            connectedBuildings.push_back(
                ConnectedBuildingPosition{newPos, BuildingOrientation::RIGHT_ANGLED});
        }
    }
    else if (dy > dx)
    {
        corner = Tile(start.x, end.y);

        uint32_t y = std::min(start.y, end.y) + 1;
        const uint32_t maxY = std::max(start.y, end.y);

        for (; y < maxY; ++y)
        {
            Tile newPos(start.x, y);
            connectedBuildings.push_back(
                ConnectedBuildingPosition{newPos, BuildingOrientation::RIGHT_ANGLED});
        }

        uint32_t x = std::min(start.x, end.x) + 1;
        const uint32_t maxX = std::max(start.x, end.x);

        for (; x < maxX; ++x)
        {
            Tile newPos(x, end.y);
            connectedBuildings.push_back(
                ConnectedBuildingPosition{newPos, BuildingOrientation::LEFT_ANGLED});
        }
    }
    else // dx == dy. Visual horizontal/virtical, logical diagonal
    {
        const int xDirection = (end.x - start.x) / (dx == 0 ? 1 : dx);
        const int yDirection = (end.y - start.y) / (dy == 0 ? 1 : dy);

        const int angle = xDirection * yDirection;

        // Moving in both direction by 1 tile. i.e. skipping start building since it will be
        // added later anyway
        int newX = start.x + xDirection;
        int newY = start.y + yDirection;

        while (newX != end.x and newY != end.y)
        {
            Tile newPos(newX, newY);

            connectedBuildings.push_back(ConnectedBuildingPosition{
                newPos,
                (angle >= 0 ? BuildingOrientation::VERTICAL : BuildingOrientation::HORIZONTAL)});

            newX += xDirection;
            newY += yDirection;
        }
    }

    connectedBuildings.push_back(ConnectedBuildingPosition{start, BuildingOrientation::CORNER});

    if (start != end)
        connectedBuildings.push_back(ConnectedBuildingPosition{end, BuildingOrientation::CORNER});

    if (start != corner and end != corner and corner.isNull() == false)
        connectedBuildings.push_back(
            ConnectedBuildingPosition{corner, BuildingOrientation::CORNER});
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
            auto [dirty, select] = m_stateMan->getComponents<CompDirty, CompSelectible>(entity);

            selection.selectedEntities.push_back(entity);
            select.isSelected = true;
            dirty.markDirty(entity);
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
    updateSelection(e.getData<EntitySelectionData>());
}

void PlayerController::updateSelection(const EntitySelectionData& newSelection)
{
    // Clear any existing selections
    for (auto& entity : m_currentEntitySelection.selection.selectedEntities)
    {
        auto [dirty, select] = m_stateMan->getComponents<CompDirty, CompSelectible>(entity);

        select.isSelected = false;
        dirty.markDirty(entity);
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
    m_stateMan->getEntities<CompUnit, CompTransform, CompDirty, CompPlayer>().each(
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
}
