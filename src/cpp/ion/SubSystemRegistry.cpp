#include "SubSystemRegistry.h"

#include <condition_variable>
#include <mutex>

using namespace ion;

SubSystemRegistry& SubSystemRegistry::getInstance()
{
    static SubSystemRegistry instance;
    return instance;
}

void SubSystemRegistry::registerSubSystem(const std::string& name,
                                          std::shared_ptr<SubSystem> component)
{
    m_subSystems[name] = std::move(component);
}

SubSystem* SubSystemRegistry::getSubSystem(const std::string& name)
{
    auto it = m_subSystems.find(name);
    if (it != m_subSystems.end())
    {
        return it->second.get();
    }
    return nullptr;
}

void SubSystemRegistry::initAll()
{
    for (auto& [name, subSystem] : m_subSystems)
    {
        subSystem->init();
    }
}

void SubSystemRegistry::shutdownAll()
{
    for (auto& [name, subSystem] : m_subSystems)
    {
        subSystem->shutdown();
    }
}