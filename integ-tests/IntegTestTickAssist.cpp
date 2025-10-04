#include "IntegTestTickAssist.h"

using namespace game;

IntegTestTickAssist::IntegTestTickAssist(/* args */)
    : m_lock(m_syncMutex, std::defer_lock)
{
    registerCallback(core::Event::Type::TICK, this, &IntegTestTickAssist::onTick);
}

IntegTestTickAssist::~IntegTestTickAssist()
{
}

void IntegTestTickAssist::onTick(const core::Event& e)
{
    std::lock_guard lock(m_syncMutex);
}

void IntegTestTickAssist::aquireLock()
{
    m_lock.lock();
}

void IntegTestTickAssist::releaseLock()
{
    m_lock.unlock();
}
