#include "Simulator.h"

#include "Event.h"
#include "GameState.h"
#include "ServiceRegistry.h"
#include "commands/CmdIdle.h"
#include "commands/CmdMove.h"
#include "components/CompAction.h"
#include "components/CompAnimation.h"
#include "components/CompBuilding.h"
#include "components/CompDirty.h"
#include "components/CompEntityInfo.h"
#include "components/CompGraphics.h"
#include "components/CompRendering.h"
#include "components/CompTransform.h"
#include "components/CompUnit.h"
#include "utils/ObjectPool.h"

#include <entt/entity/registry.hpp>

using namespace aion;

char scancodeToChar(SDL_Scancode scancode, bool shiftPressed);

Simulator::Simulator(ThreadSynchronizer<FrameData>& synchronizer,
                     std::shared_ptr<EventPublisher> publisher)
    : m_synchronizer(synchronizer),
      m_coordinates(ServiceRegistry::getInstance().getService<GameSettings>()),
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
        auto worldPos = m_coordinates.screenUnitsToFeet(m_lastMouseScreenPos);

        testBuild(worldPos, 5, Size(2, 2));
    }
    else if (scancode == SDL_SCANCODE_N)
    {
        auto worldPos = m_coordinates.screenUnitsToFeet(m_lastMouseScreenPos);

        testBuild(worldPos, 6, Size(4, 4));
    }
    else if (scancode == SDL_SCANCODE_ESCAPE)
    {
        // TODO: Not the ideal way to delete an entity
        auto [gc, dirty] = GameState::getInstance().getComponents<CompGraphics, CompDirty>(
            m_currentBuildingOnPlacement);
        gc.isDestroyed = true;
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
    auto worldPos = m_coordinates.screenUnitsToFeet(mousePos);

    if (e.getData<MouseClickData>().button == MouseClickData::Button::LEFT)
    {
        // spdlog::debug("Mouse clicked at tile position: {}", tilePos.toString());
        if (m_currentBuildingOnPlacement != entt::null)
        {
            auto& building =
                GameState::getInstance().getComponent<CompBuilding>(m_currentBuildingOnPlacement);

            if (!building.validPlacement)
            {
                GameState::getInstance().destroyEntity(m_currentBuildingOnPlacement);
            }
            // Building is permanent now, no need to track for placement
            m_currentBuildingOnPlacement = entt::null;
        }
    }
    else if (e.getData<MouseClickData>().button == MouseClickData::Button::RIGHT)
    {
        // spdlog::debug("Right mouse clicked at tile position: {}", tilePos.toString());
        resolveAction(worldPos);
    }

    if (m_isSelecting)
    {
        if (e.getData<MouseClickData>().button == MouseClickData::Button::LEFT)
        {
            m_selectionEndPosScreenUnits = e.getData<MouseClickData>().screenPosition;
            // TODO: we want on the fly selection instead of mouse button up
            onSelectingUnits(m_selectionStartPosScreenUnits, m_selectionEndPosScreenUnits);
            m_isSelecting = false;
        }
    }
}

void Simulator::onMouseButtonDown(const Event& e)
{
    if (e.getData<MouseClickData>().button == MouseClickData::Button::LEFT)
    {
        m_isSelecting = true;
        m_selectionStartPosScreenUnits = e.getData<MouseClickData>().screenPosition;
        m_selectionEndPosScreenUnits = e.getData<MouseClickData>().screenPosition;
    }
}

void Simulator::onMouseMove(const Event& e)
{
    m_lastMouseScreenPos = e.getData<MouseMoveData>().screenPos;
    if (m_currentBuildingOnPlacement != entt::null)
    {
        auto& transform =
            GameState::getInstance().getComponent<CompTransform>(m_currentBuildingOnPlacement);
        auto feet = m_coordinates.screenUnitsToFeet(m_lastMouseScreenPos);
        auto tile = m_coordinates.feetToTiles(feet);

        auto& building =
            GameState::getInstance().getComponent<CompBuilding>(m_currentBuildingOnPlacement);

        bool outOfMap = false;
        building.validPlacement = canPlaceBuildingAt(building, feet, outOfMap);

        if (!outOfMap)
        {
            // place buildings at the bottom corner of a tile
            tile += Vec2d(1, 1);
            feet = m_coordinates.tilesToFeet(tile);
            transform.position = feet;
        }

        auto& dirty =
            GameState::getInstance().getComponent<CompDirty>(m_currentBuildingOnPlacement);
        dirty.markDirty(m_currentBuildingOnPlacement);
    }
}

void Simulator::onTickStart()
{
    m_synchronizer.getSenderFrameData().frameNumber = m_frame;
    m_coordinates.setViewportPositionInPixels(
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

        if (state.hasComponent<CompAnimation>(entity))
        {
            auto animation = state.getComponent<CompAnimation>(entity);
            gc.frame = animation.frame;
        }

        if (state.hasComponent<CompAction>(entity))
        {
            auto action = state.getComponent<CompAction>(entity);
            gc.frame = action.action;
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
    }
}

void Simulator::onSelectingUnits(const Vec2d& startScreenPos, const Vec2d& endScreenPos)
{
    int selectionLeft = std::min(startScreenPos.x, endScreenPos.x);
    int selectionRight = std::max(startScreenPos.x, endScreenPos.x);
    int selectionTop = std::min(startScreenPos.y, endScreenPos.y);
    int selectionBottom = std::max(startScreenPos.y, endScreenPos.y);

    // Clear any existing selections' addons
    for (auto& entity : m_currentUnitSelection.selectedEntities)
    {
        auto& graphics = GameState::getInstance().getComponent<CompGraphics>(entity);
        graphics.addons.clear(); // TODO: We might want to selectively remove the addons
        GameState::getInstance().getComponents<CompDirty>(entity).markDirty(entity);
    }
    m_currentUnitSelection.selectedEntities.clear();

    GameState::getInstance().getEntities<CompUnit, CompTransform, CompGraphics, CompDirty>().each(
        [this, selectionLeft, selectionRight, selectionTop,
         selectionBottom](uint32_t entity, CompUnit& unit, CompTransform& transform,
                          CompGraphics& graphics, CompDirty& dirty)
        {
            auto screenPos = m_coordinates.feetToScreenUnits(transform.position);
            int unitRight = screenPos.x + transform.selectionBoxWidth / 2;
            int unitBottom = screenPos.y;
            int unitLeft = screenPos.x - transform.selectionBoxWidth / 2;
            int unitTop = screenPos.y - transform.selectionBoxHeight;

            bool intersects = !(unitRight < selectionLeft || unitLeft > selectionRight ||
                                unitBottom < selectionTop || unitTop > selectionBottom);

            if (intersects)
            {
                spdlog::info("Unit {} is selected", entity);
                m_currentUnitSelection.selectedEntities.push_back(entity);
                graphics.addons.push_back(GraphicAddon{
                    GraphicAddon::Type::CIRCLE, GraphicAddon::Circle{transform.collisionRadius}});
                dirty.markDirty(entity);
            }
        });

    if (m_currentUnitSelection.selectedEntities.size() > 0)
    {
        Event event(Event::Type::UNIT_SELECTION, UnitSelectionData{m_currentUnitSelection});
        m_publisher->publish(event);
    }
}

void Simulator::resolveAction(const Vec2d& targetFeetPos)
{
    auto tilePos = m_coordinates.feetToTiles(targetFeetPos);
    // TODO: Resolve the common action based on the unit selection here. Assuming movement for now
    testPathFinding(tilePos);
}

void aion::Simulator::testBuild(const Vec2d& targetFeetPos, int buildingType, Size size)
{
    auto& gameState = GameState::getInstance();
    auto mill = gameState.createEntity();
    auto transform = CompTransform(targetFeetPos);
    transform.face(Direction::NORTHWEST);
    gameState.addComponent(mill, transform);
    gameState.addComponent(mill, CompRendering());
    CompGraphics gc;
    gc.isStatic = true;
    gc.entityID = mill;
    gc.entityType = buildingType;
    gameState.addComponent(mill, gc);
    gameState.addComponent(mill, CompEntityInfo(buildingType));

    CompBuilding building;
    building.size = size;
    gameState.addComponent(mill, building);

    CompDirty dirty;
    dirty.markDirty(mill);
    gameState.addComponent(mill, dirty);

    m_currentBuildingOnPlacement = mill;
}

bool Simulator::canPlaceBuildingAt(const CompBuilding& building, const Vec2d& feet, bool& outOfMap)
{
    auto settings = ServiceRegistry::getInstance().getService<GameSettings>();
    auto tile = m_coordinates.feetToTiles(feet);
    auto& staticMap = GameState::getInstance().staticEntityMap;

    auto isValidTile = [&](const Vec2d& tile)
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
            if (staticMap.map[tile.x - i][tile.y - j] != 0)
            {
                return false;
            }
        }
    }
    return true;
}

void Simulator::sendGraphiInstruction(CompGraphics* instruction)
{
    m_synchronizer.getSenderFrameData().graphicUpdates.push_back(instruction);
}

void Simulator::testPathFinding(const Vec2d& end)
{
    Vec2d startPos;
    if (!m_currentUnitSelection.selectedEntities.empty())
    {
        auto& transform = GameState::getInstance().getComponent<CompTransform>(
            m_currentUnitSelection.selectedEntities[0]);
        startPos = transform.position;
    }
    else
        return;

    startPos = m_coordinates.feetToTiles(startPos);

    StaticEntityMap map = GameState::getInstance().staticEntityMap;
    std::vector<Vec2d> path =
        GameState::getInstance().getPathFinder()->findPath(map, startPos, end);

    if (path.empty())
    {
        spdlog::error("No path found from {} to {}", startPos.toString(), end.toString());
        map.map[startPos.x][startPos.y] = 2;
        map.map[end.x][end.y] = 2;
    }
    else
    {
        for (Vec2d node : path)
        {
            // map.map[node.x][node.y] = 2;
            auto entity = map.entityMap[node.x][node.y];
            if (entity != entt::null)
            {
                auto [dirty, gc] =
                    GameState::getInstance().getComponents<CompDirty, CompGraphics>(entity);
                gc.debugOverlays.push_back({DebugOverlay::Type::FILLED_CIRCLE,
                                            DebugOverlay::Color::BLUE,
                                            DebugOverlay::FixedPosition::CENTER});
                dirty.markDirty(entity);
            }
        }

        std::list<Vec2d> pathList;
        for (size_t i = 0; i < path.size(); i++)
        {
            pathList.push_back(m_coordinates.getTileCenterInFeet(path[i]));
        }

        auto move = ObjectPool<CmdMove>::acquire();
        move->path = pathList;
        move->setPriority(10); // TODO: Need a better way

        CompUnit& unit = GameState::getInstance().getComponent<CompUnit>(
            m_currentUnitSelection.selectedEntities[0]);

        if (unit.commandQueue.empty() == false)
        {
            if (move->getPriority() == unit.commandQueue.top()->getPriority())
            {
                unit.commandQueue.pop();
            }
        }
        unit.commandQueue.push(move);
    }
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