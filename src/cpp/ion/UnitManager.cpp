#include "UnitManager.h"

#include "Coordinates.h"
#include "EntityFactory.h"
#include "PlayerManager.h"
#include "commands/CmdBuild.h"
#include "commands/CmdGatherResource.h"
#include "commands/CmdMove.h"
#include "components/CompBuilding.h"
#include "components/CompResource.h"
#include "components/CompTransform.h"
#include "components/CompUnit.h"

using namespace ion;

UnitManager::UnitManager() : m_coordinates(ServiceRegistry::getInstance().getService<Coordinates>())
{
    registerCallback(Event::Type::MOUSE_BTN_UP, this, &UnitManager::onMouseButtonUp);
    registerCallback(Event::Type::MOUSE_BTN_DOWN, this, &UnitManager::onMouseButtonDown);
    registerCallback(Event::Type::ENTITY_DELETE, this, &UnitManager::onUnitDeletion);
    registerCallback(Event::Type::UNIT_REQUESTED, this, &UnitManager::onUnitRequested);
    registerCallback(Event::Type::UNIT_SELECTION, this, &UnitManager::onUnitSelection);
    registerCallback(Event::Type::BUILDING_PLACEMENT_STARTED, this,
                     &UnitManager::onBuildingPlacementStarted);
    registerCallback(Event::Type::BUILDING_PLACEMENT_FINISHED, this,
                     &UnitManager::onBuildingPlacementFinished);
}

UnitManager::~UnitManager()
{
}

void UnitManager::onBuildingPlacementStarted(const Event& e)
{
    m_buildingPlacementInProgress = true;
}

void UnitManager::onBuildingPlacementFinished(const Event& e)
{
    m_buildingPlacementInProgress = false;
}

void UnitManager::onUnitDeletion(const Event& e)
{
    auto gameState = ServiceRegistry::getInstance().getService<GameState>();

    auto entity = e.getData<EntityDeleteData>().entity;
    if (entity != entt::null && gameState->hasComponent<CompUnit>(entity))
    {
        auto [info, dirty, transform] =
            gameState->getComponents<CompEntityInfo, CompDirty, CompTransform>(entity);
        info.isDestroyed = true;
        dirty.markDirty(entity);
        gameState->gameMap.removeEntity(MapLayerType::UNITS, transform.position.toTile(), entity);
    }
}

void UnitManager::onMouseButtonUp(const Event& e)
{
    auto mousePos = e.getData<MouseClickData>().screenPosition;

    if (e.getData<MouseClickData>().button == MouseClickData::Button::LEFT)
    {
        resolveSelection(mousePos);
    }
    else if (e.getData<MouseClickData>().button == MouseClickData::Button::RIGHT)
    {
        resolveAction(mousePos);
    }
}

void UnitManager::onMouseButtonDown(const Event& e)
{
    if (e.getData<MouseClickData>().button == MouseClickData::Button::LEFT)
    {
        m_isSelecting = true;
        m_selectionStartPosScreenUnits = e.getData<MouseClickData>().screenPosition;
    }
}

void UnitManager::resolveSelection(const Vec2& screenPos)
{
    if (m_buildingPlacementInProgress)
        return;

    // Invalidate in-progress selection if mouse hasn't moved much
    if (m_isSelecting)
    {
        if ((screenPos - m_selectionStartPosScreenUnits).lengthSquared() <
            Constants::MIN_SELECTION_BOX_MOUSE_MOVEMENT)
        {
            m_isSelecting = false;
        }
    }

    // Click priorities are;
    // 1. Complete selection box
    // 2. Individual selection click

    if (m_isSelecting)
    {
        // TODO: we want on the fly selection instead of mouse button up
        completeSelectionBox(m_selectionStartPosScreenUnits, screenPos);
        m_isSelecting = false;
    }
    else
    {
        onClickToSelect(screenPos);
    }
}

void UnitManager::resolveAction(const Vec2& screenPos)
{
    auto result = whatIsAt(screenPos);
    auto target = result.entity;
    auto layer = result.layer;
    bool gatherable =
        ServiceRegistry::getInstance().getService<GameState>()->hasComponent<CompResource>(target);
    bool construction =
        ServiceRegistry::getInstance().getService<GameState>()->hasComponent<CompBuilding>(target);

    for (auto entity : m_currentUnitSelection.selectedEntities)
    {
        if (target == entt::null)
        {
            // Nothing at destination, probably request to move
            // TODO: Ensure target is within the map
            // TODO: Ensure we have control over this unit

            if (ServiceRegistry::getInstance().getService<GameState>()->hasComponent<CompUnit>(
                    entity))
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

void UnitManager::addEntitiesToSelection(const std::vector<uint32_t>& selectedEntities,
                                         UnitSelection& selection)
{
    auto gameState = ServiceRegistry::getInstance().getService<GameState>();
    for (auto entity : selectedEntities)
    {
        if (gameState->hasComponent<CompUnit>(entity)) [[likely]]
        {
            auto [dirty, select] = ServiceRegistry::getInstance()
                                       .getService<GameState>()
                                       ->getComponents<CompDirty, CompSelectible>(entity);

            selection.selectedEntities.push_back(entity);
            select.isSelected = true;
            dirty.markDirty(entity);
        }
        else if (gameState->hasComponent<CompResource>(entity)) [[likely]]
        {
            auto [dirty, select] = ServiceRegistry::getInstance()
                                       .getService<GameState>()
                                       ->getComponents<CompDirty, CompSelectible>(entity);

            selection.selectedEntities.push_back(entity);
            select.isSelected = true;
            dirty.markDirty(entity);
        }
        else [[unlikely]]
        {
            spdlog::error("Failed to select entity {}, entity is not selectible", entity);
        }
    }
}

void UnitManager::updateSelection(const UnitSelection& newSelection)
{
    // Clear any existing selections' addons
    for (auto& entity : m_currentUnitSelection.selectedEntities)
    {
        ServiceRegistry::getInstance()
            .getService<GameState>()
            ->getComponent<CompSelectible>(entity)
            .isSelected = false;
        ServiceRegistry::getInstance()
            .getService<GameState>()
            ->getComponents<CompDirty>(entity)
            .markDirty(entity);
    }
    m_currentUnitSelection = newSelection;
}

void UnitManager::onClickToSelect(const Vec2& screenPos)
{
    UnitSelection selection;

    auto result = whatIsAt(screenPos);

    // FIXME: Doesn't work for units
    if (result.entity != entt::null)
    {
        addEntitiesToSelection({result.entity}, selection);
    }
    publishEvent(Event::Type::UNIT_SELECTION, UnitSelectionData{selection});
}

UnitManager::TileMapQueryResult UnitManager::whatIsAt(const Vec2& screenPos)
{
    auto settings = ServiceRegistry::getInstance().getService<GameSettings>();
    auto gameState = ServiceRegistry::getInstance().getService<GameState>();

    auto clickedCellPos = m_coordinates->screenUnitsToTiles(screenPos);

    spdlog::debug("Clicking at grid pos {} to select", clickedCellPos.toString());

    Tile gridStartPos = clickedCellPos + Constants::MAX_SELECTION_LOOKUP_HEIGHT;
    gridStartPos.limitTo(settings->getWorldSizeInTiles().width - 1,
                         settings->getWorldSizeInTiles().height - 1);

    Tile pos;
    TileMapQueryResult result;

    for (pos.y = gridStartPos.y; pos.y >= clickedCellPos.y; pos.y--)
    {
        for (pos.x = gridStartPos.x; pos.x >= clickedCellPos.x; pos.x--)
        {
            if (gameState->gameMap.isOccupied(MapLayerType::STATIC, pos))
            {
                auto entity = gameState->gameMap.getEntity(MapLayerType::STATIC, pos);
                if (entity != entt::null)
                {
                    if (gameState->hasComponent<CompSelectible>(entity)) [[likely]]
                    {
                        auto [select, transform] =
                            gameState->getComponents<CompSelectible, CompTransform>(entity);
                        auto entityScreenPos = m_coordinates->feetToScreenUnits(transform.position);

                        const auto& boundingBox = select.getBoundingBox(transform.getDirection());
                        auto screenRect = Rect<int>(entityScreenPos.x - boundingBox.x,
                                                    entityScreenPos.y - boundingBox.y,
                                                    boundingBox.w, boundingBox.h);

                        if (screenRect.contains(screenPos))
                        {
                            return {.entity = entity, .layer = MapLayerType::STATIC};
                        }
                    }
                    else if (gameState->hasComponent<CompBuilding>(entity))
                    {
                        spdlog::debug("A building at the clicked position");
                        return {.entity = entity, .layer = MapLayerType::STATIC};
                    }
                    else [[unlikely]]
                    {
                        spdlog::error("Static entity {} at {} is not selectable", entity,
                                      pos.toString());
                    }
                }
            }
            else if (gameState->gameMap.isOccupied(MapLayerType::ON_GROUND, pos))
            {
                auto entity = gameState->gameMap.getEntity(MapLayerType::ON_GROUND, pos);
                if (entity != entt::null)
                {
                    if (gameState->hasComponent<CompBuilding>(entity))
                    {
                        spdlog::debug("A construction site at the clicked position");
                        return {.entity = entity, .layer = MapLayerType::ON_GROUND};
                    }
                }
            }
        }
    }
    return result;
}

void UnitManager::completeSelectionBox(const Vec2& startScreenPos, const Vec2& endScreenPos)
{
    // FIXME: Doing multiple drag selection seems to break the selection logic
    UnitSelection selection;

    int selectionLeft = std::min(startScreenPos.x, endScreenPos.x);
    int selectionRight = std::max(startScreenPos.x, endScreenPos.x);
    int selectionTop = std::min(startScreenPos.y, endScreenPos.y);
    int selectionBottom = std::max(startScreenPos.y, endScreenPos.y);

    auto player = ServiceRegistry::getInstance().getService<PlayerManager>()->getViewingPlayer();

    // TODO: Optimize this by using tilemap
    ServiceRegistry::getInstance()
        .getService<GameState>()
        ->getEntities<CompUnit, CompTransform, CompDirty, CompPlayer>()
        .each(
            [this, &selection, selectionLeft, selectionRight, selectionTop, selectionBottom,
             &player](uint32_t entity, CompUnit& unit, CompTransform& transform, CompDirty& dirty,
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

                    addEntitiesToSelection({entity}, selection);
                }
            });

    publishEvent(Event::Type::UNIT_SELECTION, UnitSelectionData{selection});
}

/*
    UnitManager gets unit selection changes as events instead of relying on mouse events to
    decouple selection and other selection-based activities (such as movements). Additionally
    it allows integ tests to control unit selection easily using events.
 */
void UnitManager::onUnitSelection(const Event& e)
{
    updateSelection(e.getData<UnitSelectionData>().selection);
}

void UnitManager::onUnitRequested(const Event& e)
{
    auto data = e.getData<UnitCreationData>();

    auto gameState = ServiceRegistry::getInstance().getService<GameState>();
    auto factory = ServiceRegistry::getInstance().getService<EntityFactory>();

    auto unit = factory->createEntity(data.entityType, 0);
    auto [transform, unitComp, selectible, playerComp] =
        gameState->getComponents<CompTransform, CompUnit, CompSelectible, CompPlayer>(unit);

    transform.position = data.position;
    transform.face(Direction::SOUTH);
    /*auto box = getBoundingBox(m_drs, 1388, 1);
    selectible.boundingBoxes[static_cast<int>(Direction::NONE)] = box;
    selectible.selectionIndicator = {GraphicAddon::Type::ISO_CIRCLE,
                                     GraphicAddon::IsoCircle{10, Vec2(0, 0)}};*/
    playerComp.player = data.player;

    auto newTile = transform.position.toTile();
    gameState->gameMap.addEntity(MapLayerType::UNITS, newTile, unit);

    data.player->getFogOfWar()->markAsExplored(transform.position, unitComp.lineOfSight);
}
