#include "Simulator.h"

#include "GameState.h"
#include "commands/CmdIdle.h"
#include "commands/CmdWalk.h"
#include "components/CompAction.h"
#include "components/CompUnit.h"
#include "components/CompDirty.h"
#include "components/CompEntityInfo.h"
#include "components/CompGraphics.h"
#include "components/CompRendering.h"
#include "components/CompTransform.h"
#include "utils/ObjectPool.h"

using namespace aion;

char scancodeToChar(SDL_Scancode scancode, bool shiftPressed);

void Simulator::onInit(EventLoop* eventLoop)
{
    m_messageToRenderer = ObjectPool<ThreadMessage>::acquire(ThreadMessage::Type::RENDER);
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
        auto worldPos = m_viewport.screenUnitsToFeet(mousePos);
        auto tilePos = m_viewport.feetToTiles(worldPos);

        if (e.getData<MouseClickData>().button == MouseClickData::Button::LEFT)
        {
            spdlog::debug("Mouse clicked at tile position: {}", tilePos.toString());

            m_startPosition = tilePos;
        }
        else if (e.getData<MouseClickData>().button == MouseClickData::Button::RIGHT)
        {
            spdlog::debug("Right mouse clicked at tile position: {}", tilePos.toString());

            testPathFinding(m_startPosition, tilePos);
        }
    }
    break;
    default:
        break;
    }
}

void Simulator::onTick()
{
    simulatePhysics();
    updateGraphicComponents();
    sendGraphicsInstructions();
    sendThreadMessageToRenderer();
    incrementDirtyVersion();
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

void Simulator::simulatePhysics()
{
    GameState::getInstance()
        .getEntities<CompTransform, CompAction, CompEntityInfo, CompDirty>()
        .each(
            [this](CompTransform& transform, CompAction& action,
                   CompEntityInfo& entityInfo, CompDirty& dirty)
            {
                // transform.speed = 256;
                // action.action = 1;         // Walk
                // transform.walk(1000 / 60); // TODO: Make this configurable
                // dirty.markDirty();

                // spdlog::debug("Entity {} moved to position: ({}, {})", entityInfo.entityType,
                // transform.position.x, transform.position.y);
            });
}

void Simulator::updateGraphicComponents()
{
    GameState::getInstance()
        .getEntities<CompTransform, CompEntityInfo, CompGraphics, CompDirty>()
        .each(
            [this](uint32_t entityID, CompTransform& transform,
                   CompEntityInfo& entityInfo, CompGraphics& gc, CompDirty& dirty)
            {
                // TODO: might need to optimize this later
                if (dirty.isDirty() == false)
                    return;
                gc.positionInFeet = transform.position;
                gc.direction = transform.getIsometricDirection();
                gc.variation = entityInfo.variation;
            });
}

void Simulator::sendThreadMessageToRenderer()
{
    if (!m_messageToRenderer->commandBuffer.empty())
    {
        m_rendererQueue.enqueue(m_messageToRenderer);
        m_messageToRenderer = ObjectPool<ThreadMessage>::acquire(ThreadMessage::Type::RENDER);
    }
}

void Simulator::incrementDirtyVersion()
{
    CompDirty::globalDirtyVersion++;
}

void Simulator::sendGraphiInstruction(CompGraphics* instruction)
{
    m_messageToRenderer->commandBuffer.push_back(static_cast<void*>(instruction));
}

void Simulator::testPathFinding(const Vec2d& start, const Vec2d& end)
{
    StaticEntityMap map = GameState::getInstance().staticEntityMap;
    std::vector<Vec2d> path = GameState::getInstance().getPathFinder()->findPath(map, start, end);

    if (path.empty())
    {
        spdlog::error("No path found from {} to {}", start.toString(), end.toString());
        map.map[start.x][start.y] = 2;
        map.map[end.x][end.y] = 2;
    }
    else
    {
        auto walk = ObjectPool<CmdWalk>::acquire();
        walk->path = path;
        walk->setPriority(10); // TODO: Need a better way

        // TODO: This is not correct, use selector's current selection
        GameState::getInstance().getEntities<CompUnit>().each(
            [this, &walk](uint32_t entityID, CompUnit& unit) { unit.commandQueue.push(walk); });
    }

    for (Vec2d node : path)
    {
        // map.map[node.x][node.y] = 2;
        auto entity = map.entityMap[node.x][node.y];
        if (entity != entt::null)
        {
            auto [dirty, gc] =
                GameState::getInstance().getComponents<CompDirty, CompGraphics>(entity);
            gc.debugOverlays.push_back({DebugOverlay::Type::FILLED_CIRCLE,
                                        DebugOverlay::Color::BLUE, DebugOverlay::Anchor::CENTER});
            dirty.markDirty();
        }
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