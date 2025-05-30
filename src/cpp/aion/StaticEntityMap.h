#ifndef STATICENTITYMAP_H
#define STATICENTITYMAP_H

#include <entt/entity/registry.hpp>

namespace aion
{
struct StaticEntityMap
{
    int width = 0;
    int height = 0;
    int** map = nullptr;
    uint32_t** entityMap = nullptr; // Map of entities
};
} // namespace aion

#endif