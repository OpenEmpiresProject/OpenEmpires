#include "Simulator.h"

#include "GameState.h"
#include "ObjectPool.h"
#include "components/ActionComponent.h"
#include "components/DirtyComponent.h"
#include "components/EntityInfoComponent.h"
#include "components/GraphicsComponent.h"
#include "components/TransformComponent.h"

using namespace aion;
using namespace utils;

char scancodeToChar(SDL_Scancode scancode, bool shiftPressed);
GraphicInstruction* convertTileToGraphicInstruction(entt::entity entityID,
                                                    aion::TransformComponent& transform,
                                                    aion::EntityInfoComponent& entityInfo);

void Simulator::onInit(EventLoop* eventLoop)
{
    messageToRenderer = ObjectPool<ThreadMessage>::acquire(ThreadMessage::Type::RENDER);
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
        auto worldPos = viewport.screenUnitsToFeet(mousePos);
        auto tilePos = viewport.feetToTiles(worldPos);

        if (e.getData<MouseClickData>().button == MouseClickData::Button::LEFT)
        {
            spdlog::debug("Mouse clicked at tile position: {}", tilePos.toString());

            startPosition = tilePos;
        }
        else if (e.getData<MouseClickData>().button == MouseClickData::Button::RIGHT)
        {
            spdlog::debug("Right mouse clicked at tile position: {}", tilePos.toString());

            testPathFinding(startPosition, tilePos);
        }
    }
    break;
    default:
        break;
    }
}

void Simulator::onTick()
{
    DirtyComponent::globalDirtyVersion++;
    simulatePhysics();

    if (!sentStaticInstructions)
    {
        sendStaticInstructions();
        sentStaticInstructions = true;
    }
    sendGraphicsInstructions();

    if (!messageToRenderer->commandBuffer.empty())
    {
        rendererQueue.enqueue(messageToRenderer);
        messageToRenderer = ObjectPool<ThreadMessage>::acquire(ThreadMessage::Type::RENDER);
    }
}

void aion::Simulator::sendStaticInstructions()
{
    sendStaticTileInstructions();
    sendInitialUnitsInstructions();
}

void Simulator::sendGraphicsInstructions()
{
    aion::GameState::getInstance()
        .getEntities<TransformComponent, ActionComponent, EntityInfoComponent, DirtyComponent>()
        .each(
            [this](entt::entity entity, TransformComponent& transform, ActionComponent& action,
                   EntityInfoComponent& entityInfo, DirtyComponent& dirty)
            {
                if (dirty.isDirty() == false)
                    return;

                auto gi = convertTileToGraphicInstruction(entity, transform, entityInfo);
                sendGraphiInstruction(gi);
            });
}

void Simulator::simulatePhysics()
{
    aion::GameState::getInstance()
        .getEntities<TransformComponent, ActionComponent, EntityInfoComponent, DirtyComponent>()
        .each(
            [this](TransformComponent& transform, ActionComponent& action,
                   EntityInfoComponent& entityInfo, DirtyComponent& dirty)
            {
                // transform.speed = 256;
                // action.action = 1;         // Walk
                // transform.walk(1000 / 60); // TODO: Make this configurable
                // dirty.markDirty();

                // spdlog::debug("Entity {} moved to position: ({}, {})", entityInfo.entityType,
                // transform.position.x, transform.position.y);
            });
}

void aion::Simulator::sendStaticTileInstructions()
{
    auto message = ObjectPool<ThreadMessage>::acquire(ThreadMessage::Type::RENDER);

    aion::GameState::getInstance()
        .getEntities<aion::TransformComponent, aion::EntityInfoComponent>()
        .each(
            [this, &message](entt::entity entityID, aion::TransformComponent& transform,
                             aion::EntityInfoComponent& entityInfo)
            {
                // TODO: This works only for testing. Should send these instruction based on the map
                if (entityInfo.entityType == 2)
                {
                    auto graphicInstruction =
                        convertTileToGraphicInstruction(entityID, transform, entityInfo);
                    auto tilePos = transform.getTilePosition();
                    if (GameState::getInstance().staticEntityMap.map[tilePos.x][tilePos.y] != 0)
                    {
                        entityInfo.setDebugHighlightType(utils::DebugHighlightType::TILE_TREE_MARK);
                        graphicInstruction->debugHighlightType = entityInfo.debugHighlightType;
                    }
                    message->commandBuffer.push_back(static_cast<void*>(graphicInstruction));
                }
                else if (entityInfo.entityType == 4) // tree
                {
                    auto graphicInstruction = ObjectPool<GraphicInstruction>::acquire();
                    graphicInstruction->type = GraphicInstruction::Type::ADD;
                    graphicInstruction->entity = entityID;
                    graphicInstruction->positionInFeet = transform.position;
                    graphicInstruction->entityType = entityInfo.entityType;
                    graphicInstruction->variation = entityInfo.variation;
                    graphicInstruction->isStatic = entityInfo.isStatic;
                    message->commandBuffer.push_back(static_cast<void*>(graphicInstruction));
                }
            });

    if (message->commandBuffer.empty())
        ObjectPool<ThreadMessage>::release(message);
    else
        rendererQueue.enqueue(message);
}

void aion::Simulator::sendInitialUnitsInstructions()
{
    auto message = ObjectPool<ThreadMessage>::acquire(ThreadMessage::Type::RENDER);

    aion::GameState::getInstance()
        .getEntities<aion::TransformComponent, aion::EntityInfoComponent, ActionComponent>()
        .each(
            [this, &message](entt::entity entityID, aion::TransformComponent& transform,
                             aion::EntityInfoComponent& entityInfo, ActionComponent& action)
            {
                auto graphicInstruction = ObjectPool<GraphicInstruction>::acquire();
                graphicInstruction->type = GraphicInstruction::Type::ADD;
                graphicInstruction->entity = entityID;
                graphicInstruction->positionInFeet = transform.position;
                graphicInstruction->entityType = entityInfo.entityType;
                graphicInstruction->direction = transform.getIsometricDirection();
                graphicInstruction->action = action.action;

                message->commandBuffer.push_back(static_cast<void*>(graphicInstruction));
            });

    if (message->commandBuffer.empty())
        ObjectPool<ThreadMessage>::release(message);
    else
        rendererQueue.enqueue(message);
}

void aion::Simulator::sendGraphiInstruction(GraphicInstruction* instruction)
{
    messageToRenderer->commandBuffer.push_back(static_cast<void*>(instruction));
}

void aion::Simulator::testPathFinding(const Vec2d& start, const Vec2d& end)
{
    StaticEntityMap map = GameState::getInstance().staticEntityMap;
    std::vector<Vec2d> path =
        GameState::getInstance().getPathFinder()->findPath(map, start, end);

    if (path.empty())
    {
        spdlog::error("No path found from {} to {}", start.toString(),
        end.toString());
        map.map[start.x][start.y] = 2;
        map.map[end.x][end.y] = 2;
    }

    for (Vec2d node : path)
    {
        // map.map[node.x][node.y] = 2;
        auto entity = map.entityMap[node.x][node.y];
        if (entity != entt::null)
        {
            auto [transform, entityInfo] =
                GameState::getInstance()
                    .getComponents<TransformComponent, EntityInfoComponent>(entity);
            auto gi = convertTileToGraphicInstruction(entity, transform, entityInfo);
            entityInfo.setDebugHighlightType(utils::DebugHighlightType::TILE_CIRCLE);
            gi->debugHighlightType = entityInfo.debugHighlightType;
            sendGraphiInstruction(gi);
        }
    }
}

GraphicInstruction* convertTileToGraphicInstruction(entt::entity entityID,
                                                    aion::TransformComponent& transform,
                                                    aion::EntityInfoComponent& entityInfo)
{
    if (entityInfo.entityType == 2)
    {
        auto graphicInstruction = ObjectPool<GraphicInstruction>::acquire();
        graphicInstruction->type = GraphicInstruction::Type::ADD;
        graphicInstruction->entity = entityID;
        graphicInstruction->positionInFeet = transform.position;
        graphicInstruction->entityType = entityInfo.entityType;
        graphicInstruction->direction = transform.getIsometricDirection();
        graphicInstruction->variation = entityInfo.variation;
        graphicInstruction->isStatic = entityInfo.isStatic;
        return graphicInstruction;
    }
    return nullptr;
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