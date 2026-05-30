#include "CmdAttack.h"

#include "CmdMeleeAttack.h"
#include "CmdRangeAttack.h"
#include "debug.h"

using namespace core;

void CmdAttack::onQueue()
{
    if (m_stateMan->hasComponent<CompMeleeAttack>(m_entityID))
    {
        auto cmd = ObjectPool<CmdMeleeAttack>::acquire();
        cmd->target = target;
        m_attackCommand = cmd;
    }
    else if (m_stateMan->hasComponent<CompRangeAttack>(m_entityID))
    {
        auto cmd = ObjectPool<CmdRangeAttack>::acquire();
        cmd->target = target;
        m_attackCommand = cmd;
    }
    else
    {
        debug_assert(false,
                     "Attack command is valid only on either melee or range units. {} is neither",
                     m_entityID);
    }
    m_attackCommand->setEntityID(m_entityID);
    m_attackCommand->init();
    m_attackCommand->setPriority(getPriority());
    m_attackCommand->onQueue();
}

void CmdAttack::onStart()
{
    m_attackCommand->onStart();
}

bool CmdAttack::onExecute(int deltaTimeMs, int currentTick, std::list<Command*>& subCommands)
{
    return m_attackCommand->onExecute(deltaTimeMs, currentTick, subCommands);
}

std::string CmdAttack::toString() const
{
    return m_attackCommand->toString();
}

core::Command* CmdAttack::clone()
{
    return ObjectPool<CmdAttack>::acquire(*this);
}

void CmdAttack::destroy()
{
    ObjectPool<CmdAttack>::release(this);
    if (m_stateMan->hasComponent<CompMeleeAttack>(m_entityID))
    {
        ObjectPool<CmdMeleeAttack>::release((CmdMeleeAttack*) m_attackCommand);
    }
    else if (m_stateMan->hasComponent<CompRangeAttack>(m_entityID))
    {
        ObjectPool<CmdRangeAttack>::release((CmdRangeAttack*) m_attackCommand);
    }
}
