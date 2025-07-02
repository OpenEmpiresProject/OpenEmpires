#include "CommandCenter.h"

#include "GameState.h"
#include "commands/Command.h"
#include "components/CompUnit.h"
#include "utils/Logger.h"
#include "utils/Types.h"

using namespace ion;

CommandCenter::CommandCenter()
{
}

CommandCenter::~CommandCenter()
{
}

void CommandCenter::onInit(EventLoop* eventLoop)
{
}

void CommandCenter::onExit()
{
}

void CommandCenter::onEvent(const Event& e)
{
    if (e.type == Event::Type::TICK)
    {
        Command::incrementTotalTicks();
        std::list<Command*> newCommands;
        GameState::getInstance().getEntities<CompUnit>().each(
            [this, &newCommands, &e](uint32_t entity, CompUnit& unit)
            {
                if (!unit.commandQueue.empty())
                {
                    auto cmd = unit.commandQueue.top();
                    auto completed = cmd->onExecute(e.getData<TickData>().deltaTimeMs, newCommands);

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

                        if (!unit.commandQueue.empty())
                        {
                            auto nextCmd = unit.commandQueue.top();
                            nextCmd->onStart();
                        }
                    }
                }
            });
    }
    else if (e.type == Event::Type::COMMAND_REQUEST)
    {
        auto data = e.getData<CommandRequestData>();
        data.command->setPriority(Command::DEFAULT_PRIORITY + Command::CHILD_PRIORITY_OFFSET);
        data.command->setEntityID(data.entity);

        CompUnit& unit = GameState::getInstance().getComponent<CompUnit>(data.entity);

        // Remove all the components except the default one (i.e. idle)
        while (unit.commandQueue.size() > 1)
        {
            if (unit.commandQueue.top()->getPriority() > Command::DEFAULT_PRIORITY)
            {
                unit.commandQueue.pop();
            }
        }
        unit.commandQueue.push(data.command);
        data.command->onQueue();
    }
}