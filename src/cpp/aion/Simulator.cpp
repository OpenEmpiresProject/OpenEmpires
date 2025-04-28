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

void Simulator::onInit(EventLoop* eventLoop)
{
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
}

void aion::Simulator::sendStaticInstructions()
{
    sendStaticTileInstructions();
    sendInitialUnitsInstructions();
}

void Simulator::sendGraphicsInstructions()
{
    auto message = ObjectPool<ThreadMessage>::acquire(ThreadMessage::Type::RENDER);

    aion::GameState::getInstance()
        .getEntities<TransformComponent, ActionComponent, EntityInfoComponent, DirtyComponent>()
        .each(
            [this, &message](entt::entity entityID, TransformComponent& transform,
                             ActionComponent& action, EntityInfoComponent& entityInfo,
                             DirtyComponent& dirty)
            {
                // spdlog::debug("Entity {} moved to position: ({}, {})", transform.position.x,
                // transform.position.y);
                if (dirty.isDirty() == false)
                    return;

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

void Simulator::simulatePhysics()
{
    aion::GameState::getInstance()
        .getEntities<TransformComponent, ActionComponent, EntityInfoComponent, DirtyComponent>()
        .each(
            [this](TransformComponent& transform, ActionComponent& action,
                   EntityInfoComponent& entityInfo, DirtyComponent& dirty)
            {
                transform.speed = 256;
                action.action = 1;         // Walk
                transform.walk(1000 / 60); // TODO: Make this configurable
                dirty.markDirty();

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
                    auto graphicInstruction = ObjectPool<GraphicInstruction>::acquire();
                    graphicInstruction->type = GraphicInstruction::Type::ADD;
                    graphicInstruction->entity = entityID;
                    graphicInstruction->positionInFeet = transform.position;
                    graphicInstruction->entityType = entityInfo.entityType;
                    graphicInstruction->direction = transform.getIsometricDirection();
                    graphicInstruction->variation = rand() % 50 + 1;
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
