#include "SubSystemRegistry.h"

#include <condition_variable>
#include <mutex>

using namespace aion;

SubSystemRegistry& SubSystemRegistry::getInstance()
{
    static SubSystemRegistry instance;
    return instance;
}

void SubSystemRegistry::registerSubSystem(const std::string& name,
                                          std::unique_ptr<SubSystem> component)
{
    subSystems[name] = std::move(component);
}

SubSystem* SubSystemRegistry::getSubSystem(const std::string& name)
{
    auto it = subSystems.find(name);
    if (it != subSystems.end())
    {
        return it->second.get();
    }
    return nullptr;
}

void SubSystemRegistry::initAll()
{
    for (auto& [name, subSystem] : subSystems)
    {
        subSystem->init();
    }
}

void SubSystemRegistry::shutdownAll()
{
    for (auto& [name, subSystem] : subSystems)
    {
        subSystem->shutdown();
    }
}

void aion::SubSystemRegistry::waitForAll()
{
    std::mutex mtx;
    std::unique_lock<std::mutex> lock(mtx);
    std::condition_variable cv;

    // Wait forever
    cv.wait(lock, [] { return false; });
}
