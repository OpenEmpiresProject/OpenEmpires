#include "SubSystemRegistry.h"

#include <condition_variable>
#include <mutex>

using namespace core;

SubSystemRegistry& SubSystemRegistry::getInstance()
{
    static SubSystemRegistry instance;
    return instance;
}

void SubSystemRegistry::registerSubSystem(const std::string& name, Ref<SubSystem> component)
{
    m_subSystems[name] = std::move(component);
}

Ref<SubSystem> SubSystemRegistry::getSubSystem(const std::string& name)
{
    auto it = m_subSystems.find(name);
    if (it != m_subSystems.end())
    {
        return it->second;
    }
    return nullptr;
}

void SubSystemRegistry::initAll()
{
    for (auto& [name, subSystem] : m_subSystems)
    {
        subSystem->init();
    }
    m_allInitialized = true;
}

void SubSystemRegistry::shutdownAll()
{
    for (auto& [name, subSystem] : m_subSystems)
    {
        subSystem->shutdown();
    }
}

bool SubSystemRegistry::isAllInitialized() const
{
    return m_allInitialized;
}
