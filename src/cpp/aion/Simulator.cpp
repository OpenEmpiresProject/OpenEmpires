#include "Simulator.h"

#include "GameState.h"
#include "ObjectPool.h"
#include "components/ActionComponent.h"
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
    simulatePhysics();

    if (!sentStaticInstructions)
    {
        sendStaticInstructions();
        sentStaticInstructions = true;
    }
    sendGraphicsInstructions();
}

void Simulator::sendGraphicsInstructions()
{
   
}

void aion::Simulator::sendStaticInstructions()
{
    auto message = ObjectPool<ThreadMessage>::acquire(ThreadMessage::Type::RENDER);

    auto view = aion::GameState::getInstance()
                    .getEntities<aion::TransformComponent, aion::EntityInfoComponent>();

    aion::GameState::getInstance()
        .getEntities<aion::TransformComponent, aion::EntityInfoComponent>()
        .each(
            [this, &message](entt::entity entityID, aion::TransformComponent& transform,
                                aion::EntityInfoComponent& entityInfo)
            {
                auto graphicInstruction = ObjectPool<GraphicInstruction>::acquire();
                graphicInstruction->type = GraphicInstruction::Type::ADD;
                graphicInstruction->entity = entityID;
                graphicInstruction->positionInFeet = transform.position;
                graphicInstruction->entityType = entityInfo.entityType;
                graphicInstruction->direction = transform.getDirection();
                // graphicInstruction->entitySubType = entityInfo.entitySubType;

                // TODO: Temporary hack to randomize grass tiles
                if (entityInfo.entityType == 2)
                {
                    graphicInstruction->variation = rand() % 50 + 1;
                }

                message->commandBuffer.push_back(static_cast<void*>(graphicInstruction));
            });

    

    if (message->commandBuffer.empty())
        ObjectPool<ThreadMessage>::release(message);
    else
        rendererQueue.enqueue(message);
}

void Simulator::simulatePhysics()
{
    // aion::GameState::getInstance().getEntities<aion::TransformComponent>().each(
    //     [this](aion::TransformComponent& transform)
    //     {
    //         transform.position.x += 1; // Example physics simulation: move right
    //     });
}
