#ifndef GRIDNMAP_H
#define GRIDNMAP_H

#include "Vec2d.h"
#include "debug.h"
#include "utils/Logger.h"

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

    /**
     * @brief Checks if a cell at the specified grid position is occupied in the given map layer.
     *
     * This function verifies whether the cell located at the provided grid position (`gridPos`)
     * within the specified map layer (`layerType`) is occupied. It first asserts that the layer
     * type is valid, then checks if the grid position is within the bounds of the map. If the
     * position is valid, it returns the occupancy status of the cell. If the position is invalid,
     * it logs an error and returns false.
     *
     * @param layerType The type of the map layer to check.
     * @param gridPos The 2D grid position to check for occupancy.
     * @return true if the cell is occupied; false otherwise or if the position is invalid.
     */
    inline bool isOccupied(MapLayerType layerType, const Vec2d& gridPos) const
    {
        debug_assert(layerType < MapLayerType::MAX_LAYERS, "Invalid layer type {}",
                     (int) layerType);

        if (gridPos.x >= 0 && gridPos.y >= 0 && gridPos.x < width && gridPos.y < height) [[likely]]
        {
            return layers[layerType].cells[gridPos.x][gridPos.y].isOccupied();
        }
        else [[unlikely]]
        {
            spdlog::error("Invalid grid position: ({}, {})", gridPos.x, gridPos.y);
            return false;
        }
    }

    /**
     * @brief Adds an entity to the specified map layer at the given grid position.
     *
     * This function inserts the provided entity into the cell located at `gridPos`
     * within the specified `layerType`. It performs bounds checking to ensure that
     * the grid position is valid and that the layer type is within the allowed range.
     * If the position or layer type is invalid, an error is logged.
     *
     * @param layerType The type of map layer to which the entity should be added.
     * @param gridPos The 2D grid position where the entity will be placed.
     * @param entity The unique identifier of the entity to add.
     */
    void addEntity(MapLayerType layerType, const Vec2d& gridPos, uint32_t entity)
    {
        // debug_assert(layerType < MapLayerType::MAX_LAYERS, "Invalid layer type {}", layerType);

        if (gridPos.x >= 0 && gridPos.y >= 0 && gridPos.x < width && gridPos.y < height) [[likely]]
        {
            layers[layerType].cells[gridPos.x][gridPos.y].addEntity(entity);
        }
        else [[unlikely]]
        {
            spdlog::error("Invalid grid position: ({}, {}) to add entity {}", gridPos.x, gridPos.y,
                          entity);
        }
    }

    /**
     * Retrieves the entity ID at the specified grid position and layer.
     *
     * @param layerType The type of map layer to query.
     * @param gridPos The 2D grid position to look up.
     * @return The entity ID at the given position and layer, or entt::null if the position is
     * invalid.
     *
     * Logs an error if the grid position is out of bounds.
     * Asserts that the layerType is valid.
     */
    uint32_t getEntity(MapLayerType layerType, const Vec2d& gridPos) const
    {
        // debug_assert(layerType < MapLayerType::MAX_LAYERS, "Invalid layer type {}", layerType);

        if (gridPos.x >= 0 && gridPos.y >= 0 && gridPos.x < width && gridPos.y < height) [[likely]]
        {
            return layers[layerType].cells[gridPos.x][gridPos.y].getEntity();
        }
        else [[unlikely]]
        {
            spdlog::error("Invalid grid position: ({}, {})", gridPos.x, gridPos.y);
            return entt::null;
        }
    }

    /**
     * @brief Initializes the grid map with the specified width and height.
     *
     * Allocates memory for all map layers and their corresponding cells.
     * Each layer is initialized with a 2D array of MapCell objects. Not
     * performing this in a constructor intentionally to make this object
     * shallow-copy-able and light weight. Therefore, obtaining copies of
     * this object is safe to manipulate the same underlying grid map.
     *
     * @param width The width of the grid map.
     * @param height The height of the grid map.
     */
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