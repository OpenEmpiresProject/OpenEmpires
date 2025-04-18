#ifndef GAMESTATE_H
#define GAMESTATE_H

#include <unordered_map>
#include "Constants.h"
#include "Component.h"

namespace spice
{
    class ComponentStorage
    {
    public:
        ComponentStorage() = default;
        ~ComponentStorage() = default;

        template <typename ComponentType>
        class ComponentIterator
        {
        public:
            ComponentIterator(std::byte* ptr, size_t index) : ptr(ptr), index(index) {}

            ComponentType& operator*() { return *reinterpret_cast<ComponentType*>(ptr); }
            ComponentIterator& operator++()
            {
                ptr += sizeof(ComponentType);
                ++index;
                return *this;
            }
            bool operator!=(const ComponentIterator& other) const { return index != other.index; }

        private:
            std::byte* ptr;
            size_t index;
        };

        bool hasComponent(int componentType) const
        {
            return components.find(componentType) != components.end();
        }

        template <typename ComponentType>
        void registerComponent()
        {
            static_assert(std::is_base_of<Component<ComponentType>, ComponentType>::value, "ComponentType must derive from spice::Component");
            components[ComponentType::type()] = std::vector<std::byte>(sizeof(ComponentType) * utils::Constants::MAX_ENTITIES);
        }

        template <typename ComponentType>
        ComponentType* getComponent(int entityId)
        {
            auto it = components.find(ComponentType::type());
            if (it != components.end() && entityId < utils::Constants::MAX_ENTITIES)
            {
                return reinterpret_cast<ComponentType*>(&it->second[entityId * sizeof(ComponentType)]);
            }
            return nullptr;
        }

        template <typename ComponentType>
        ComponentIterator<ComponentType> begin()
        {
            auto it = components.find(ComponentType::type());
            if (it != components.end())
            {
                return ComponentIterator<ComponentType>(it->second.data(), 0);
            }
            return ComponentIterator<ComponentType>(nullptr, 0);
        }

        template <typename ComponentType>
        ComponentIterator<ComponentType> end()
        {
            auto it = components.find(ComponentType::type());
            if (it != components.end())
            {
                return ComponentIterator<ComponentType>(it->second.data() + it->second.size(), utils::Constants::MAX_ENTITIES);
            }
            return ComponentIterator<ComponentType>(nullptr, 0);
        }

    private:
        std::unordered_map<int, std::vector<std::byte>> components;
    };
}


#endif