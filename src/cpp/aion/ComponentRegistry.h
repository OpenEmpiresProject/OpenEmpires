#ifndef COMPONENTREGISTRY_H
#define COMPONENTREGISTRY_H

#include <memory>
#include <unordered_map>
#include <string>
#include <typeindex>
#include <type_traits>
#include "Component.h"

namespace aion
{
    class ComponentRegistry
    {
    public:
        static ComponentRegistry &getInstance();

        void registerComponent(const std::string &name, std::unique_ptr<Component> component);
        Component *getComponent(const std::string &name);
        void initAll();
        void shutdownAll();
        void waitForAll();

    private:
        ComponentRegistry() = default;
        ~ComponentRegistry() = default;

        ComponentRegistry(const ComponentRegistry &) = delete;
        ComponentRegistry &operator=(const ComponentRegistry &) = delete;
        ComponentRegistry(ComponentRegistry &&) = delete;
        ComponentRegistry &operator=(ComponentRegistry &&) = delete;

        std::unordered_map<std::string, std::unique_ptr<Component>> components;
    };
}

#endif