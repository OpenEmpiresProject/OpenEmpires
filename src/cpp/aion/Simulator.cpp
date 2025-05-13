#include "Simulator.h"

#include "GameState.h"
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
    m_synchronizer.getSenderFrameData().frameNumber = m_frame;
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

void Simulator::sendGraphiInstruction(CompGraphics* instruction)
{
    m_synchronizer.getSenderFrameData().graphicUpdates.push_back(instruction);
}

void Simulator::testPathFinding(const Vec2d& start, const Vec2d& end)
{
    Vec2d startPos;
    GameState::getInstance().getEntities<CompUnit, CompTransform>().each(
        [this, &startPos](uint32_t entityID, CompUnit& unit, CompTransform& transofrm)
        { startPos = transofrm.position; });

    startPos = m_viewport.feetToTiles(startPos);

    StaticEntityMap map = GameState::getInstance().staticEntityMap;
    std::vector<Vec2d> path =
        GameState::getInstance().getPathFinder()->findPath(map, startPos, end);

    if (path.empty())
    {
        spdlog::error("No path found from {} to {}", startPos.toString(), end.toString());
        map.map[start.x][start.y] = 2;
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
            pathList.push_back(m_viewport.getTileCenterInFeet(path[i]));
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