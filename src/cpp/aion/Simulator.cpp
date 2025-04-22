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
    switch (e.getType())
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
    // simulatePhysics();
    // sendGraphicsInstructions();
}

void Simulator::sendGraphicsInstructions()
{
    // spdlog::debug("Sending graphics instructions...");

    // aion::GameState::getInstance()
    //     .getEntities<aion::ActionComponent, aion::TransformComponent,
    //     aion::EntityInfoComponent>() .each(
    //         [this, &message](entt::entity entityID, aion::ActionComponent& action,
    //                                  aion::TransformComponent& transform,
    //                                  aion::EntityInfoComponent& entityInfo)
    //         {
    //             GraphicsID graphicsID(entityInfo.entityType, action.action,
    //                                   0, // TODO: FIX this
    //                                   transform.getDirection());

    //            auto graphicInstruction = ObjectPool<GraphicInstruction>::acquire(
    //                GraphicInstruction::Type::ADD,
    //                entityID,
    //                graphicsID,
    //                transform.position);

    //            message->commandBuffer.push_back(static_cast<void*>(graphicInstruction));
    //        });

    static bool done = false;

    if (!done)
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
                    GraphicsID graphicsID(entityInfo.entityType, 0,
                                          0, // TODO: FIX this
                                          transform.getDirection());

                    auto graphicInstruction = ObjectPool<GraphicInstruction>::acquire(
                        GraphicInstruction::Type::ADD, entityID, graphicsID, transform.position);

                    message->commandBuffer.push_back(static_cast<void*>(graphicInstruction));
                });

        if (message->commandBuffer.empty())
            ObjectPool<ThreadMessage>::release(message);
        else
            rendererQueue.enqueue(message);
        done = true;
    }
}

void Simulator::simulatePhysics()
{
    // aion::GameState::getInstance().getEntities<aion::TransformComponent>().each(
    //     [this](aion::TransformComponent& transform)
    //     {
    //         transform.position.x += 1; // Example physics simulation: move right
    //     });
}
