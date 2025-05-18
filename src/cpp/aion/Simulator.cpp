#include "Simulator.h"

#include "Event.h"
#include "GameState.h"
#include "ServiceRegistry.h"
#include "commands/CmdIdle.h"
#include "commands/CmdMove.h"
#include "components/CompAction.h"
#include "components/CompAnimation.h"
#include "components/CompDirty.h"
#include "components/CompEntityInfo.h"
#include "components/CompGraphics.h"
#include "components/CompRendering.h"
#include "components/CompTransform.h"
#include "components/CompUnit.h"
#include "components/CompBuilding.h"
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
}

void Simulator::onInit(EventLoop* eventLoop)
{
    ObjectPool<ThreadMessage>::reserve(1000);
    ObjectPool<CompGraphics>::reserve(1000);
}

void Simulator::onExit()
{
}

void Simulator::onEvent(const Event& e)
{
    switch (e.type)
    {
    case Event::Type::TICK:
        onTick();
        break;
    case Event::Type::KEY_UP:
    {
        SDL_Scancode scancode = static_cast<SDL_Scancode>(e.getData<KeyboardData>().keyCode);

        // TODO: temporary
        if (scancode == SDL_SCANCODE_B)
        {
            auto worldPos = m_coordinates.screenUnitsToFeet(m_lastMouseScreenPos);

            testBuildMill(worldPos, 5, Size(2, 2));
        } 
        else if (scancode == SDL_SCANCODE_N)
        {
            auto worldPos = m_coordinates.screenUnitsToFeet(m_lastMouseScreenPos);

            testBuildMill(worldPos, 6, Size(4, 4));
        }
        else if (scancode == SDL_SCANCODE_ESCAPE)
        {
            GameState::getInstance().destroyEntity(m_currentBuildingOnPlacement);
            m_currentBuildingOnPlacement = entt::null;
        }        
    }
    break;
    case Event::Type::MOUSE_BTN_UP:
    {
        auto mousePos = e.getData<MouseClickData>().screenPosition;
        auto worldPos = m_coordinates.screenUnitsToFeet(mousePos);

        if (e.getData<MouseClickData>().button == MouseClickData::Button::LEFT)
        {
            // spdlog::debug("Mouse clicked at tile position: {}", tilePos.toString());
            if (m_currentBuildingOnPlacement != entt::null)
            {
                auto& building = GameState::getInstance().getComponent<CompBuilding>(
                    m_currentBuildingOnPlacement);

                if (building.cantPlace)
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
    break;
    case Event::Type::MOUSE_BTN_DOWN:
    {
        if (e.getData<MouseClickData>().button == MouseClickData::Button::LEFT)
        {
            m_isSelecting = true;
            m_selectionStartPosScreenUnits = e.getData<MouseClickData>().screenPosition;
            m_selectionEndPosScreenUnits = e.getData<MouseClickData>().screenPosition;
        }
        break;
    }
    case Event::Type::MOUSE_MOVE:
    {
        m_lastMouseScreenPos = e.getData<MouseMoveData>().screenPos;
        if (m_currentBuildingOnPlacement != entt::null)
        {
            auto& transform = GameState::getInstance().getComponent<CompTransform>(m_currentBuildingOnPlacement);
            auto feet = m_coordinates.screenUnitsToFeet(m_lastMouseScreenPos);
            auto tile = m_coordinates.feetToTiles(feet);

            auto& building =
                GameState::getInstance().getComponent<CompBuilding>(m_currentBuildingOnPlacement);

            auto& staticMap = GameState::getInstance().staticEntityMap;
            auto settings = ServiceRegistry::getInstance().getService<GameSettings>();

            building.cantPlace = false;

            if (tile.x <= 1 || tile.y <= 1 || tile.x >= (settings->getWorldSize().width - 2) ||
                tile.y >= (settings->getWorldSize().height - 2))
                building.cantPlace = true;
            else
            {
                for (size_t i = 0; i < building.size.width; i++)
                {
                    for (size_t j = 0; j < building.size.height; j++)
                    {
                        if (staticMap.map[tile.x - i][tile.y - j] != 0)
                        {
                            building.cantPlace = true;
                            break;
                        }
                    }
                }
            }

            // place buildings almost (10, 10 feet before) at the bottom corner of a tile
            tile += Vec2d(1, 1);
            feet = m_coordinates.tilesToFeet(tile);
            //feet = m_coordinates.tilesToFeet(tile) - Vec2d(10, 10);

            transform.position = feet;

            auto& dirty =
                GameState::getInstance().getComponent<CompDirty>(m_currentBuildingOnPlacement);
            dirty.markDirty();

        }
        break;
    }
    default:
        break;
    }
}

void Simulator::onTick()
{
    onTickStart();
    updateGraphicComponents();
    sendGraphicsInstructions();
    onTickEnd();
}

void Simulator::onTickStart()
{
    m_synchronizer.getSenderFrameData().frameNumber = m_frame;
    m_coordinates.setViewportPositionInPixels(
        m_synchronizer.getSenderFrameData().viewportPositionInPixels);

}

void Simulator::onTickEnd()
{
    CompDirty::globalDirtyVersion++;
    m_frame++;
    m_synchronizer.waitForReceiver();
}

void Simulator::sendGraphicsInstructions()
{
    GameState::getInstance()
        .getEntities<CompTransform, CompEntityInfo, CompDirty, CompGraphics>()
        .each(
            [this](uint32_t entity, CompTransform& transform, CompEntityInfo& entityInfo,
                   CompDirty& dirty, CompGraphics& gc)
            {
                if (dirty.isDirty() == false)
                    return;

                auto instruction = ObjectPool<CompGraphics>::acquire();
                *instruction = gc;
                sendGraphiInstruction(instruction);
            });
}

void Simulator::updateGraphicComponents()
{
    GameState::getInstance()
        .getEntities<CompTransform, CompEntityInfo, CompGraphics, CompDirty>()
        .each(
            [this](uint32_t entityID, CompTransform& transform, CompEntityInfo& entityInfo,
                   CompGraphics& gc, CompDirty& dirty)
            {
                // TODO: might need to optimize this later
                if (dirty.isDirty() == false)
                    return;
                gc.positionInFeet = transform.position;
                gc.direction = transform.getIsometricDirection();
                gc.variation = entityInfo.variation;
            });

    GameState::getInstance().getEntities<CompGraphics, CompDirty, CompAnimation>().each(
        [this](uint32_t entityID, CompGraphics& gc, CompDirty& dirty, CompAnimation& animation)
        {
            // TODO: might need to optimize this later
            if (dirty.isDirty() == false)
                return;
            gc.frame = animation.frame;
        });

    GameState::getInstance().getEntities<CompGraphics, CompDirty, CompAction>().each(
        [this](uint32_t entityID, CompGraphics& gc, CompDirty& dirty, CompAction& action)
        {
            // TODO: might need to optimize this later
            if (dirty.isDirty() == false)
                return;
            gc.action = action.action;
        });

     GameState::getInstance().getEntities<CompGraphics, CompDirty, CompBuilding>().each(
        [this](uint32_t entityID, CompGraphics& gc, CompDirty& dirty, CompBuilding& building)
        {
            // TODO: might need to optimize this later
            if (dirty.isDirty() == false)
                return;
            if (building.cantPlace)
                gc.shading = Color::RED;
            else
                gc.shading = Color::NONE;

            gc.landSize = building.size;
        });
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
        GameState::getInstance().getComponents<CompDirty>(entity).markDirty();
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
                dirty.markDirty();
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

void aion::Simulator::testBuildMill(const Vec2d& targetFeetPos, int buildingType, Size size)
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
    dirty.markDirty();
    gameState.addComponent(mill, dirty);

    m_currentBuildingOnPlacement = mill;
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
                dirty.markDirty();
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