#ifndef SUBSYSTEMREGISTRY_H
#define SUBSYSTEMREGISTRY_H

#include "SubSystem.h"

#include <memory>
#include <string>
#include <type_traits>
#include <typeindex>
#include <unordered_map>

namespace core
{
class SubSystemRegistry
{
  public:
    static SubSystemRegistry& getInstance();

    void registerSubSystem(const std::string& name, std::shared_ptr<SubSystem> subSystem);
    SubSystem* getSubSystem(const std::string& name);
    void initAll();
    void shutdownAll();
    bool isAllInitialized() const;

  private:
    SubSystemRegistry() = default;
    ~SubSystemRegistry() = default;

    SubSystemRegistry(const SubSystemRegistry&) = delete;
    SubSystemRegistry& operator=(const SubSystemRegistry&) = delete;
    SubSystemRegistry(SubSystemRegistry&&) = delete;
    SubSystemRegistry& operator=(SubSystemRegistry&&) = delete;

    std::unordered_map<std::string, std::shared_ptr<SubSystem>> m_subSystems;

    bool m_allInitialized = false;
};
} // namespace core

#endif