#include "Simulator.h"

#include "Event.h"
#include "GameState.h"
#include "ServiceRegistry.h"
#include "commands/CmdIdle.h"
#include "commands/CmdWalk.h"
#include "components/CompAction.h"
#include "components/CompAnimation.h"
#include "components/CompDirty.h"
#include "components/CompEntityInfo.h"
#include "components/CompGraphics.h"
#include "components/CompRendering.h"
#include "components/CompTransform.h"
#include "components/CompUnit.h"
#include "utils/ObjectPool.h"

using namespace aion;

char scancodeToChar(SDL_Scancode scancode, bool shiftPressed);

Simulator::Simulator(ThreadSynchronizer<FrameData>& synchronizer,
                     std::shared_ptr<EventPublisher> publisher)
    : m_synchronizer(synchronizer),
      m_coordinates(ServiceRegistry::getInstance().getService<GameSettings>()),
      m_publisher(std::move(publisher))
{
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
        // spdlog::debug("Key up event: {}", scancodeToChar(scancode, false));
    }
    break;
    case Event::Type::MOUSE_BTN_UP:
    {
        // spdlog::debug("Mouse button up event: {} at {}",
        //                 e.getData<MouseClickData>().button == MouseClickData::Button::LEFT ?
        //                 "Left" : "Right", e.getData<MouseClickData>().screenPosition.toString());

        auto mousePos = e.getData<MouseClickData>().screenPosition;
        // TODO: This is not thread safe.
        auto worldPos = m_coordinates.screenUnitsToFeet(mousePos);

        if (e.getData<MouseClickData>().button == MouseClickData::Button::LEFT)
        {
            // spdlog::debug("Mouse clicked at tile position: {}", tilePos.toString());
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
    }
    break;
    default:
        break;
    }
}

void Simulator::onTick()
{
    m_synchronizer.getSenderFrameData().frameNumber = m_frame;
    m_coordinates.setViewportPositionInPixels(
        m_synchronizer.getSenderFrameData().viewportPositionInPixels);
    // spdlog::info("Simulating frame {}", m_frame);

    updateGraphicComponents();
    sendGraphicsInstructions();
    incrementDirtyVersion();
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
}

void Simulator::incrementDirtyVersion()
{
    CompDirty::globalDirtyVersion++;
}

void aion::Simulator::onSelectingUnits(const Vec2d& startScreenPos, const Vec2d& endScreenPos)
{
    int selectionLeft = std::min(startScreenPos.x, endScreenPos.x);
    int selectionRight = std::max(startScreenPos.x, endScreenPos.x);
    int selectionTop = std::min(startScreenPos.y, endScreenPos.y);
    int selectionBottom = std::max(startScreenPos.y, endScreenPos.y);

    m_currentUnitSelection.selectedEntities.clear();

    GameState::getInstance().getEntities<CompUnit, CompTransform>().each(
        [this, selectionLeft, selectionRight, selectionTop,
         selectionBottom](uint32_t entity, CompUnit& unit, CompTransform& transform)
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
                m_currentUnitSelection.selectedEntities.push_back(entity);
                spdlog::info("Unit {} is selected", entity);
            }
        });

    if (m_currentUnitSelection.selectedEntities.size() > 0)
    {
        Event event(Event::Type::UNIT_SELECTION, UnitSelectionData{m_currentUnitSelection});
        m_publisher->publish(event);
    }
}

void aion::Simulator::resolveAction(const Vec2d& targetFeetPos)
{
    auto tilePos = m_coordinates.feetToTiles(targetFeetPos);
    // TODO: Resolve the common action based on the unit selection here. Assuming movement for now
    testPathFinding(tilePos);
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
                                            DebugOverlay::Anchor::CENTER});
                dirty.markDirty();
            }
        }

        std::list<Vec2d> pathList;
        for (size_t i = 0; i < path.size(); i++)
        {
            pathList.push_back(m_coordinates.getTileCenterInFeet(path[i]));
        }

        auto walk = ObjectPool<CmdWalk>::acquire();
        walk->path = pathList;
        walk->setPriority(10); // TODO: Need a better way

        // TODO: This is not correct, use selector's current selection
        GameState::getInstance().getEntities<CompUnit>().each(
            [this, &walk](uint32_t entityID, CompUnit& unit)
            {
                if (unit.commandQueue.empty() == false)
                {
                    if (walk->getPriority() == unit.commandQueue.top()->getPriority())
                    {
                        unit.commandQueue.pop();
                    }
                }
                unit.commandQueue.push(walk);
            });
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