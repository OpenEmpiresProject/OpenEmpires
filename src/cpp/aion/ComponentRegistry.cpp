#include "ComponentRegistry.h"
#include <condition_variable>
#include <mutex>

using namespace aion;

ComponentRegistry& ComponentRegistry::getInstance()
{
    static ComponentRegistry instance;
    return instance;
}

void ComponentRegistry::registerComponent(const std::string& name, std::unique_ptr<Component> component)
{
    components[name] = std::move(component);
}

Component* ComponentRegistry::getComponent(const std::string& name)
{
    auto it = components.find(name);
    if (it != components.end())
    {
        return it->second.get();
    }
    return nullptr;
}

void ComponentRegistry::initAll()
{
    for (auto& [name, component] : components)
    {
        component->init();
    }
}

void ComponentRegistry::shutdownAll()
{
    for (auto& [name, component] : components)
    {
        component->shutdown();
    }
}

void aion::ComponentRegistry::waitForAll()
{
    std::mutex mtx;
    std::unique_lock<std::mutex> lock(mtx);
    std::condition_variable cv;

    // Wait forever
    cv.wait(lock, [] { return false; });
}
