#include "Simulator.h"

#include "GameState.h"
#include "components/ActionComponent.h"
#include "components/EntityInfoComponent.h"
#include "components/GraphicsComponent.h"
#include "components/TransformComponent.h"

using namespace aion;

void Simulator::onInit() {}

void Simulator::onExit() {}

void Simulator::onEvent(const Event &e)
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
    simulatePhysics();
    sendGraphicsInstructions();
}

void Simulator::sendGraphicsInstructions()
{
    std::vector<GraphicInstruction> instructionList;

    aion::GameState::getInstance()
        .getEntities<aion::ActionComponent, aion::TransformComponent, aion::EntityInfoComponent>()
        .each(
            [this, &instructionList](entt::entity entityID, aion::ActionComponent &action,
                                     aion::TransformComponent &transform,
                                     aion::EntityInfoComponent &entityInfo)
            {
                GraphicsID graphicsID(entityInfo.entityType, action.action,
                                      0, // TODO: FIX this
                                      transform.getDirection());

                GraphicInstruction instruction(GraphicInstruction::Type::ADD, entityID, graphicsID,
                                               transform.position);

                instructionList.push_back(instruction);
                rendererQueue.enqueue(instructionList);
            });
}

void Simulator::simulatePhysics()
{
    aion::GameState::getInstance().getEntities<aion::TransformComponent>().each(
        [this](aion::TransformComponent &transform)
        {
            transform.position.x += 1; // Example physics simulation: move right
        });
}
