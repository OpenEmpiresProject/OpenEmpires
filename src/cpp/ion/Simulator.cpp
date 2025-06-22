#include "Simulator.h"

#include "Event.h"
#include "GameState.h"
#include "ServiceRegistry.h"
#include "UI.h"
#include "commands/CmdGatherResource.h"
#include "commands/CmdIdle.h"
#include "commands/CmdMove.h"
#include "components/CompAction.h"
#include "components/CompAnimation.h"
#include "components/CompBuilding.h"
#include "components/CompDirty.h"
#include "components/CompEntityInfo.h"
#include "components/CompGraphics.h"
#include "components/CompRendering.h"
#include "components/CompResource.h"
#include "components/CompSelectible.h"
#include "components/CompTransform.h"
#include "components/CompUIElement.h"
#include "components/CompUnit.h"
#include "components/CompPlayer.h"
#include "PlayerManager.h"
#include "utils/ObjectPool.h"

#include <algorithm>
#include <entt/entity/registry.hpp>

using namespace ion;

char scancodeToChar(SDL_Scancode scancode, bool shiftPressed);

Simulator::Simulator(ThreadSynchronizer<FrameData>& synchronizer,
                     std::shared_ptr<EventPublisher> publisher)
    : m_synchronizer(synchronizer),
      m_coordinates(ServiceRegistry::getInstance().getService<Coordinates>()),
      m_publisher(std::move(publisher))
{
    m_currentBuildingOnPlacement = entt::null;
    CompDirty::g_dirtyEntities.reserve(10000);

    registerCallback(Event::Type::TICK, this, &Simulator::onTick);
    registerCallback(Event::Type::KEY_UP, this, &Simulator::onKeyUp);
    registerCallback(Event::Type::KEY_DOWN, this, &Simulator::onKeyDown);
    registerCallback(Event::Type::MOUSE_BTN_UP, this, &Simulator::onMouseButtonUp);
    registerCallback(Event::Type::MOUSE_BTN_DOWN, this, &Simulator::onMouseButtonDown);
    registerCallback(Event::Type::MOUSE_MOVE, this, &Simulator::onMouseMove);
    registerCallback(Event::Type::UNIT_SELECTION, this, &Simulator::onUnitSelection);
}

void Simulator::onInit(EventLoop* eventLoop)
{
    ObjectPool<ThreadMessage>::reserve(1000);
    ObjectPool<CompGraphics>::reserve(1000);
}

void Simulator::onTick(const Event& e)
{
    onTickStart();
    updateGraphicComponents();
    sendGraphicsInstructions();
    onTickEnd();
}

void Simulator::onKeyUp(const Event& e)
{
    SDL_Scancode scancode = static_cast<SDL_Scancode>(e.getData<KeyboardData>().keyCode);

    // TODO: temporary
    if (scancode == SDL_SCANCODE_B)
    {
        auto worldPos = m_coordinates->screenUnitsToFeet(m_lastMouseScreenPos);

        testBuild(worldPos, 5, Size(2, 2));
    }
    else if (scancode == SDL_SCANCODE_N)
    {
        auto worldPos = m_coordinates->screenUnitsToFeet(m_lastMouseScreenPos);

        testBuild(worldPos, 6, Size(4, 4));
    }
    else if (scancode == SDL_SCANCODE_ESCAPE)
    {
        // TODO: Not the ideal way to delete an entity
        auto [info, dirty] = GameState::getInstance().getComponents<CompEntityInfo, CompDirty>(
            m_currentBuildingOnPlacement);
        info.isDestroyed = true;
        dirty.markDirty(m_currentBuildingOnPlacement);
        GameState::getInstance().destroyEntity(m_currentBuildingOnPlacement);
        m_currentBuildingOnPlacement = entt::null;
    }
}

void Simulator::onKeyDown(const Event& e)
{
}

void Simulator::onMouseButtonUp(const Event& e)
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

void Simulator::onMouseButtonDown(const Event& e)
{
    if (e.getData<MouseClickData>().button == MouseClickData::Button::LEFT)
    {
        m_isSelecting = true;
        m_selectionStartPosScreenUnits = e.getData<MouseClickData>().screenPosition;
    }
}

void Simulator::onMouseMove(const Event& e)
{
    m_lastMouseScreenPos = e.getData<MouseMoveData>().screenPos;
    if (m_currentBuildingOnPlacement != entt::null)
    {
        auto& transform =
            GameState::getInstance().getComponent<CompTransform>(m_currentBuildingOnPlacement);
        auto feet = m_coordinates->screenUnitsToFeet(m_lastMouseScreenPos);
        auto tile = feet.toTile();

        auto& building =
            GameState::getInstance().getComponent<CompBuilding>(m_currentBuildingOnPlacement);

        bool outOfMap = false;
        building.validPlacement = canPlaceBuildingAt(building, feet, outOfMap);

        if (!outOfMap)
        {
            // place buildings at the bottom corner of a tile
            tile += Tile(1, 1);
            feet = tile.toFeet();
            transform.position = feet - Feet(10, 10);
        }

        auto& dirty =
            GameState::getInstance().getComponent<CompDirty>(m_currentBuildingOnPlacement);
        dirty.markDirty(m_currentBuildingOnPlacement);
    }
}

void Simulator::onTickStart()
{
    m_synchronizer.getSenderFrameData().frameNumber = m_frame;
    m_coordinates->setViewportPositionInPixels(
        m_synchronizer.getSenderFrameData().viewportPositionInPixels);

    if (!m_initialized)
    {
        GameState::getInstance().getEntities<CompDirty>().each(
            [this](uint32_t entity, CompDirty& dirty) { dirty.markDirty(entity); });
        m_initialized = true;
    }
}

void Simulator::onTickEnd()
{
    CompDirty::globalDirtyVersion++;
    m_frame++;
    m_synchronizer.waitForReceiver([&]() { onSynchorizedBlock(); });
    CompDirty::g_dirtyEntities.clear();
}

void Simulator::onSynchorizedBlock()
{
    // TODO: Make this work, might need to delay destroying entities
    // GameState::getInstance().destroyAllPendingEntities();
}

void Simulator::onUnitSelection(const Event& e)
{
    auto selectedEntities = e.getData<UnitSelectionData>().selection.selectedEntities;
    if (selectedEntities.size() == 1)
    {
        auto resourceEntity = selectedEntities[0];

        if (GameState::getInstance().hasComponent<CompResource>(resourceEntity))
        {
            auto resource = GameState::getInstance().getComponent<CompResource>(resourceEntity);
            spdlog::info("Selected entity has {} resources", resource.resource.amount);
        }
    }
}

void Simulator::sendGraphicsInstructions()
{
    auto& state = GameState::getInstance();

    for (auto entity : CompDirty::g_dirtyEntities)
    {
        auto& gc = state.getComponent<CompGraphics>(entity);
        auto instruction = ObjectPool<CompGraphics>::acquire();
        *instruction = gc;
        sendGraphiInstruction(instruction);
    }
}

void Simulator::updateGraphicComponents()
{
    auto& state = GameState::getInstance();
    for (auto entity : CompDirty::g_dirtyEntities)
    {
        auto [transform, entityInfo, gc] =
            state.getComponents<CompTransform, CompEntityInfo, CompGraphics>(entity);

        gc.positionInFeet = transform.position;
        gc.direction = transform.getIsometricDirection();
        gc.variation = entityInfo.variation;
        gc.entitySubType = entityInfo.entitySubType;
        gc.entityType = entityInfo.entityType;
        gc.isDestroyed = entityInfo.isDestroyed;

        if (state.hasComponent<CompSelectible>(entity))
        {
            auto select = state.getComponent<CompSelectible>(entity);
            if (select.isSelected)
            {
                // TODO: Respect other addons
                gc.addons = {select.selectionIndicator};
            }
            else
            {
                gc.addons.clear();
            }
        }

        if (state.hasComponent<CompAnimation>(entity))
        {
            auto animation = state.getComponent<CompAnimation>(entity);
            gc.frame = animation.frame;
        }

        if (state.hasComponent<CompAction>(entity))
        {
            auto action = state.getComponent<CompAction>(entity);
            gc.action = action.action;
        }

        if (state.hasComponent<CompBuilding>(entity))
        {
            auto building = state.getComponent<CompBuilding>(entity);
            if (building.validPlacement)
                gc.shading = Color::NONE;
            else
                gc.shading = Color::RED;

            gc.landSize = building.size;
        }

        if (state.hasComponent<CompUIElement>(entity))
        {
            gc.positionInFeet = Feet::null;
            gc.positionInScreenUnits = {transform.position.x, transform.position.y};

            auto ui = state.getComponent<CompUIElement>(entity);
            if (ui.type == UIRenderingType::TEXT)
            {
                GraphicAddon addon{GraphicAddon::Type::TEXT, GraphicAddon::Text{ui.text}};
                gc.addons = {addon};
            }
        }
    }
}

void Simulator::onSelectingUnits(const Vec2& startScreenPos, const Vec2& endScreenPos)
{
    clearSelection();

    int selectionLeft = std::min(startScreenPos.x, endScreenPos.x);
    int selectionRight = std::max(startScreenPos.x, endScreenPos.x);
    int selectionTop = std::min(startScreenPos.y, endScreenPos.y);
    int selectionBottom = std::max(startScreenPos.y, endScreenPos.y);

    GameState::getInstance().getEntities<CompUnit, CompTransform, CompDirty>().each(
        [this, selectionLeft, selectionRight, selectionTop, selectionBottom](
            uint32_t entity, CompUnit& unit, CompTransform& transform, CompDirty& dirty)
        {
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

                addEntitiesToSelection({entity});
            }
        });

    if (m_currentUnitSelection.selectedEntities.size() > 0)
    {
        Event event(Event::Type::UNIT_SELECTION, UnitSelectionData{m_currentUnitSelection});
        m_publisher->publish(event);
    }
}

void Simulator::onClickToSelect(const Vec2& screenPos)
{
    clearSelection();

    auto entity = whatIsAt(screenPos);

    if (entity != entt::null)
    {
        addEntitiesToSelection({entity});
    }
}

void Simulator::resolveSelection(const Vec2& screenPos)
{
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
    // 1. Any building placement (you can't select anything if there is planned building)
    // 2. Complete selection box
    // 3. Individual selection click

    if (m_currentBuildingOnPlacement != entt::null)
    {
        auto [building, transform] = GameState::getInstance().getComponents<CompBuilding,
                                                                             CompTransform>(
            m_currentBuildingOnPlacement);

        if (!building.validPlacement)
        {
            GameState::getInstance().destroyEntity(m_currentBuildingOnPlacement);
        }

        publishEvent(Event::Type::BUILDING_PLACED,
                     BuildingPlacedData{m_currentBuildingOnPlacement, transform.position.toTile()});

        // Building is permanent now, no need to track for placement
        m_currentBuildingOnPlacement = entt::null;
    }
    else if (m_isSelecting)
    {
        // TODO: we want on the fly selection instead of mouse button up
        onSelectingUnits(m_selectionStartPosScreenUnits, screenPos);
        m_isSelecting = false;
    }
    else
    {
        onClickToSelect(screenPos);
    }
}

void Simulator::resolveAction(const Vec2& screenPos)
{
    auto target = whatIsAt(screenPos);
    bool gatherable = GameState::getInstance().hasComponent<CompResource>(target);

    for (auto entity : m_currentUnitSelection.selectedEntities)
    {
        if (target == entt::null)
        {
            // Nothing at destination, probably request to move
            // TODO: Ensure target is within the map
            // TODO: Ensure we have control over this unit

            if (GameState::getInstance().hasComponent<CompUnit>(entity))
            {
                auto worldPos = m_coordinates->screenUnitsToFeet(screenPos);
                auto cmd = ObjectPool<CmdMove>::acquire();
                cmd->goal = worldPos;

                Event event(Event::Type::COMMAND_REQUEST, CommandRequestData{cmd, entity});
                m_publisher->publish(event);
            }
        }
        else if (gatherable)
        {
            // It is a request to gather the resource at target
            // TODO: Check selected entity's capability to gather, if it isn't doable, fall back to
            // move
            auto cmd = ObjectPool<CmdGatherResource>::acquire();
            cmd->target = target;

            Event event(Event::Type::COMMAND_REQUEST, CommandRequestData{cmd, entity});
            m_publisher->publish(event);
        }
    }
}

void Simulator::testBuild(const Feet& targetFeetPos, int buildingType, Size size)
{
    auto& gameState = GameState::getInstance();
    auto mill = gameState.createEntity();
    auto transform = CompTransform(targetFeetPos);
    transform.face(Direction::NORTHWEST);
    gameState.addComponent(mill, transform);
    gameState.addComponent(mill, CompRendering());
    CompGraphics gc;
    gc.entityID = mill;
    gc.entityType = buildingType;
    gc.layer = GraphicLayer::ENTITIES;
    gameState.addComponent(mill, gc);
    gameState.addComponent(mill, CompEntityInfo(buildingType));

    auto playerManager = ServiceRegistry::getInstance().getService<PlayerManager>();
    auto player = playerManager->getPlayer(0); // TODO - replace with current player on the UI
    gameState.addComponent(mill, CompPlayer{player});

    CompBuilding building;
    building.size = size;
    building.lineOfSight = 256 * 5;

    gameState.addComponent(mill, building);

    CompDirty dirty;
    dirty.markDirty(mill);
    gameState.addComponent(mill, dirty);

    m_currentBuildingOnPlacement = mill;
}

bool Simulator::canPlaceBuildingAt(const CompBuilding& building, const Feet& feet, bool& outOfMap)
{
    auto settings = ServiceRegistry::getInstance().getService<GameSettings>();
    auto tile = feet.toTile();
    auto staticMap = GameState::getInstance().gameMap.getMap(MapLayerType::STATIC);

    auto isValidTile = [&](const Tile& tile)
    {
        return tile.x >= 0 && tile.y >= 0 && tile.x < settings->getWorldSizeInTiles().width &&
               tile.y < settings->getWorldSizeInTiles().height;
    };

    outOfMap = false;

    for (int i = 0; i < building.size.width; i++)
    {
        for (int j = 0; j < building.size.height; j++)
        {
            if (!isValidTile({tile.x - i, tile.y - j}))
            {
                outOfMap = true;
                return false;
            }
            if (staticMap[tile.x - i][tile.y - j].isOccupied())
            {
                return false;
            }
        }
    }
    return true;
}

void Simulator::addEntitiesToSelection(const std::vector<uint32_t>& selectedEntities)
{
    auto& gameState = GameState::getInstance();
    for (auto entity : selectedEntities)
    {
        if (gameState.hasComponent<CompSelectible>(entity)) [[likely]]
        {
            auto [dirty, select] =
                GameState::getInstance().getComponents<CompDirty, CompSelectible>(entity);

            m_currentUnitSelection.selectedEntities.push_back(entity);
            select.isSelected = true;
            dirty.markDirty(entity);
        }
        else [[unlikely]]
        {
            spdlog::error("Failed to select entity {}, entity is not selectible", entity);
        }
    }
    // TODO: Might be able to optimize this
    Event event(Event::Type::UNIT_SELECTION, UnitSelectionData{m_currentUnitSelection});
    m_publisher->publish(event);
}

void Simulator::clearSelection()
{
    // Clear any existing selections' addons
    for (auto& entity : m_currentUnitSelection.selectedEntities)
    {
        GameState::getInstance().getComponent<CompSelectible>(entity).isSelected = false;
        GameState::getInstance().getComponents<CompDirty>(entity).markDirty(entity);
    }
    m_currentUnitSelection.selectedEntities.clear();
}

uint32_t Simulator::whatIsAt(const Vec2& screenPos)
{
    auto settings = ServiceRegistry::getInstance().getService<GameSettings>();
    auto& gameState = GameState::getInstance();

    auto clickedCellPos = m_coordinates->screenUnitsToTiles(screenPos);

    spdlog::debug("Clicking at grid pos {} to select", clickedCellPos.toString());

    Tile gridStartPos = clickedCellPos + Constants::MAX_SELECTION_LOOKUP_HEIGHT;
    gridStartPos.limitTo(settings->getWorldSizeInTiles().width - 1,
                         settings->getWorldSizeInTiles().height - 1);

    Tile pos;

    for (pos.y = gridStartPos.y; pos.y >= clickedCellPos.y; pos.y--)
    {
        for (pos.x = gridStartPos.x; pos.x >= clickedCellPos.x; pos.x--)
        {
            if (gameState.gameMap.isOccupied(MapLayerType::STATIC, pos))
            {
                auto entity = gameState.gameMap.getEntity(MapLayerType::STATIC, pos);
                if (entity != entt::null)
                {
                    if (gameState.hasComponent<CompSelectible>(entity)) [[likely]]
                    {
                        auto [select, transform] =
                            gameState.getComponents<CompSelectible, CompTransform>(entity);
                        auto entityScreenPos = m_coordinates->feetToScreenUnits(transform.position);

                        const auto& boundingBox = select.getBoundingBox(transform.getDirection());
                        auto screenRect = Rect<int>(entityScreenPos.x - boundingBox.x,
                                                    entityScreenPos.y - boundingBox.y,
                                                    boundingBox.w, boundingBox.h);

                        if (screenRect.contains(screenPos))
                        {
                            return entity;
                        }
                    }
                    else [[unlikely]]
                    {
                        spdlog::error("Static entity {} at {} is not selectable", entity,
                                      pos.toString());
                    }
                }
            }
            // TODO: Check entity layer as well
        }
    }
    return entt::null;
}

void Simulator::sendGraphiInstruction(CompGraphics* instruction)
{
    m_synchronizer.getSenderFrameData().graphicUpdates.push_back(instruction);
}

char scancodeToChar(SDL_Scancode scancode, bool shiftPressed)
{
    // Handle letters (A-Z)
    if (scancode >= SDL_SCANCODE_A && scancode <= SDL_SCANCODE_Z)
    {
        return shiftPressed ? 'A' + (scancode - SDL_SCANCODE_A) : 'a' + (scancode - SDL_SCANCODE_A);
    }

    // Handle numbers (0–9)
    if (scancode >= SDL_SCANCODE_1 && scancode <= SDL_SCANCODE_0)
    {
        if (!shiftPressed)
        {
            static const char numMap[] = {'1', '2', '3', '4', '5', '6', '7', '8', '9', '0'};
            return numMap[(scancode - SDL_SCANCODE_1) % 10];
        }
        else
        {
            static const char shiftNumMap[] = {'!', '@', '#', '$', '%', '^', '&', '*', '(', ')'};
            return shiftNumMap[(scancode - SDL_SCANCODE_1) % 10];
        }
    }

    // Special cases (US QWERTY)
    static const std::unordered_map<SDL_Scancode, std::pair<char, char>> specialMap = {
        {SDL_SCANCODE_SPACE, {' ', ' '}},        {SDL_SCANCODE_MINUS, {'-', '_'}},
        {SDL_SCANCODE_EQUALS, {'=', '+'}},       {SDL_SCANCODE_LEFTBRACKET, {'[', '{'}},
        {SDL_SCANCODE_RIGHTBRACKET, {']', '}'}}, {SDL_SCANCODE_BACKSLASH, {'\\', '|'}},
        {SDL_SCANCODE_SEMICOLON, {';', ':'}},    {SDL_SCANCODE_APOSTROPHE, {'\'', '"'}},
        {SDL_SCANCODE_GRAVE, {'`', '~'}},        {SDL_SCANCODE_COMMA, {',', '<'}},
        {SDL_SCANCODE_PERIOD, {'.', '>'}},       {SDL_SCANCODE_SLASH, {'/', '?'}}};

    auto it = specialMap.find(scancode);
    if (it != specialMap.end())
    {
        return shiftPressed ? it->second.second : it->second.first;
    }

    return '\0'; // Unhandled key
}