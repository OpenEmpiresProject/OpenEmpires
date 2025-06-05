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
                    auto completed = cmd->onExecute(entity, e.getData<TickData>().deltaTimeMs);
                    if (completed)
                    {
                        spdlog::debug("Entity {}'s command {} completed.", entity, cmd->toString());
                        unit.commandQueue.pop();
                        cmd->destroy();
                    }

                    if (cmd->onCreateSubCommands(newCommands))
                    {
                        for (auto subCmd : newCommands)
                        {
                            unit.commandQueue.push(subCmd);
                        }
                        newCommands.clear();
                    }
                }
            });
    }
}