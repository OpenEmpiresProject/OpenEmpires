#include "CommandCenter.h"

#include "StateManager.h"
#include "commands/Command.h"
#include "components/CompUnit.h"
#include "utils/Logger.h"
#include "utils/Types.h"

using namespace core;

CommandCenter::CommandCenter()
{
    registerCallback(Event::Type::TICK, this, &CommandCenter::onTick);
    registerCallback(Event::Type::COMMAND_REQUEST, this, &CommandCenter::onCommandRequest);
}

void CommandCenter::onCommandRequest(const Event& e)
{
    auto data = e.getData<CommandRequestData>();
    if (data.command->getPriority() == -1)
        data.command->setPriority(Command::DEFAULT_PRIORITY + Command::CHILD_PRIORITY_OFFSET);
    data.command->setEntityID(data.entity);

    CompUnit& unit =
        ServiceRegistry::getInstance().getService<StateManager>()->getComponent<CompUnit>(
            data.entity);

    // Remove all the components except the default one (i.e. idle)
    while (unit.commandQueue.size() > 1)
    {
        debug_assert(unit.commandQueue.top()->getPriority() >= 0,
                     "Command {} for entity {} has invalid priority set",
                     unit.commandQueue.top()->toString(), unit.commandQueue.top()->getEntityID());

        if (unit.commandQueue.top()->getPriority() > Command::DEFAULT_PRIORITY)
        {
            unit.commandQueue.pop();
        }
    }
    unit.commandQueue.push(data.command);
    data.command->onQueue();
}

void CommandCenter::onTick(const Event& e)
{
    std::list<Command*> newCommands;
    ServiceRegistry::getInstance().getService<StateManager>()->getEntities<CompUnit>().each(
        [this, &newCommands, &e](uint32_t entity, CompUnit& unit)
        {
            if (!unit.commandQueue.empty())
            {
                auto cmd = unit.commandQueue.top();
                if (cmd->isExecutedAtLeastOnce() == false)
                {
                    cmd->onStart();
                    cmd->setExecutedAtLeastOnce(true);
                }

                auto tickData = e.getData<TickData>();
                auto completed =
                    cmd->onExecute(tickData.deltaTimeMs, tickData.currentTick, newCommands);

                for (auto subCmd : newCommands)
                {
                    subCmd->setEntityID(entity);
                    unit.commandQueue.push(subCmd);
                    subCmd->onQueue();
                }
                newCommands.clear();

                if (completed)
                {
                    spdlog::debug("Entity {}'s command {} completed.", entity, cmd->toString());
                    unit.commandQueue.pop();
                    cmd->destroy();
                }
            }
        });
}