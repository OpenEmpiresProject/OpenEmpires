#ifndef SUBSYSTEMREGISTRY_H
#define SUBSYSTEMREGISTRY_H

#include "SubSystem.h"

#include <memory>
#include <string>
#include <type_traits>
#include <typeindex>
#include <unordered_map>

namespace aion
{
class SubSystemRegistry
{
  public:
    static SubSystemRegistry &getInstance();

    void registerSubSystem(const std::string &name, std::unique_ptr<SubSystem> subSystem);
    SubSystem *getSubSystem(const std::string &name);
    void initAll();
    void shutdownAll();
    void waitForAll();

  private:
    SubSystemRegistry() = default;
    ~SubSystemRegistry() = default;

    SubSystemRegistry(const SubSystemRegistry &) = delete;
    SubSystemRegistry &operator=(const SubSystemRegistry &) = delete;
    SubSystemRegistry(SubSystemRegistry &&) = delete;
    SubSystemRegistry &operator=(SubSystemRegistry &&) = delete;

    std::unordered_map<std::string, std::unique_ptr<SubSystem>> subSystems;
};
} // namespace aion

#endif