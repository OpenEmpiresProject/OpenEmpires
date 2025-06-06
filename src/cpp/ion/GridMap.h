#ifndef GRIDNMAP_H
#define GRIDNMAP_H

#include <entt/entity/registry.hpp>
#include <vector>

namespace ion
{
struct MapCell
{
    std::vector<uint32_t> entities; // List of entities occupying this cell

    inline bool isOccupied() const
    {
        return !entities.empty();
    }

    void addEntity(uint32_t entity)
    {
        entities.push_back(entity);
    }

    uint32_t getEntity() const
    {
        return entities.front();
    }

    void removeEntity(uint32_t entity)
    {
        entities.erase(std::remove(entities.begin(), entities.end(), entity), entities.end());
    }

    void removeAllEntities()
    {
        entities.clear();
    }
};

struct MapLayer
{
    MapCell** cells = nullptr;
};

enum MapLayerType
{
    GROUND = 0,
    STATIC,
    UNITS,

    // Add more layers here

    MAX_LAYERS
};

struct GridMap
{
    uint32_t width = 0;
    uint32_t height = 0;
    MapLayer* layers = nullptr;

    MapCell** getMap(MapLayerType layerType)
    {
        return layers[layerType].cells;
    }

    MapCell** getStaticMap()
    {
        return layers[MapLayerType::STATIC].cells;
    }

    MapCell** getGroundMap()
    {
        return layers[MapLayerType::GROUND].cells;
    }

    void init(uint32_t width, uint32_t height)
    {
        this->width = width;
        this->height = height;
        layers = new MapLayer[static_cast<size_t>(MapLayerType::MAX_LAYERS)];
        for (size_t i = 0; i < static_cast<size_t>(MapLayerType::MAX_LAYERS); i++)
        {
            auto& layer = layers[i];

            layer.cells = new MapCell*[width];
            for (uint32_t i = 0; i < width; ++i)
            {
                layer.cells[i] = new MapCell[height];
            }
        }
    }
};

} // namespace ion

#endif