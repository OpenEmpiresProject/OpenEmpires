#ifndef TILEMAP_H
#define TILEMAP_H

#include "Tile.h"
#include "debug.h"
#include "utils/Constants.h"
#include "utils/Logger.h"
#include "utils/Types.h"

#include <algorithm>
#include <entt/entity/registry.hpp>
#include <list>

namespace ion
{
struct MapCell
{
    std::list<uint32_t> entities; // List of entities occupying this cell

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
        entities.remove(entity);
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

enum class MapLayerType
{
    GROUND = 0,
    GROUND_DECOS, // walkable decorators on the ground
    STATIC,       // doesn't move, not walkable
    UNITS,
    // Add more layers here

    MAX_LAYERS
};

struct TileMap
{
    uint32_t width = 0;
    uint32_t height = 0;
    MapLayer* layers = nullptr;

    MapCell** getMap(MapLayerType layerType)
    {
        return layers[toInt(layerType)].cells;
    }

    MapCell** getStaticMap()
    {
        return layers[toInt(MapLayerType::STATIC)].cells;
    }

    MapCell** getGroundMap()
    {
        return layers[toInt(MapLayerType::GROUND)].cells;
    }

    /**
     * @brief Checks if a cell at the specified grid position is occupied in the given map layer.
     *
     * This function verifies whether the cell located at the provided grid position (`pos`)
     * within the specified map layer (`layerType`) is occupied. It first asserts that the layer
     * type is valid, then checks if the grid position is within the bounds of the map. If the
     * position is valid, it returns the occupancy status of the cell. If the position is invalid,
     * it logs an error and returns false.
     *
     * @param layerType The type of the map layer to check.
     * @param pos The 2D grid position to check for occupancy.
     * @return true if the cell is occupied; false otherwise or if the position is invalid.
     */
    inline bool isOccupied(MapLayerType layerType, const Tile& pos) const
    {
        auto layerTypeInt = toInt(layerType);

        if (pos.x >= 0 && pos.y >= 0 && pos.x < width && pos.y < height) [[likely]]
        {
            return layers[layerTypeInt].cells[pos.x][pos.y].isOccupied();
        }
        else [[unlikely]]
        {
            spdlog::error("Invalid grid position: ({}, {})", pos.x, pos.y);
            return false;
        }
    }

    inline bool isOccupiedByAnother(MapLayerType layerType,
                                    const Tile& pos,
                                    uint32_t myEntity) const
    {
        auto layerTypeInt = toInt(layerType);

        if (pos.x >= 0 && pos.y >= 0 && pos.x < width && pos.y < height) [[likely]]
        {
            if (layers[layerTypeInt].cells[pos.x][pos.y].isOccupied())
            {
                auto& entities = layers[layerTypeInt].cells[pos.x][pos.y].entities;

                // Check if a value other than myEntity exist in the entities list
                return std::any_of(entities.begin(), entities.end(),
                                   [myEntity](uint32_t entity) { return entity != myEntity; });
            }
        }
        else [[unlikely]]
        {
            spdlog::error("Invalid grid position: ({}, {})", pos.x, pos.y);
            return false;
        }
    }

    /**
     * @brief Adds an entity to the specified map layer at the given grid position.
     *
     * This function inserts the provided entity into the cell located at `pos`
     * within the specified `layerType`. It performs bounds checking to ensure that
     * the grid position is valid and that the layer type is within the allowed range.
     * If the position or layer type is invalid, an error is logged.
     *
     * @param layerType The type of map layer to which the entity should be added.
     * @param pos The 2D grid position where the entity will be placed.
     * @param entity The unique identifier of the entity to add.
     */
    void addEntity(MapLayerType layerType, const Tile& pos, uint32_t entity)
    {
        auto layerTypeInt = toInt(layerType);

        if (pos.x >= 0 && pos.y >= 0 && pos.x < width && pos.y < height) [[likely]]
        {
            layers[layerTypeInt].cells[pos.x][pos.y].addEntity(entity);
        }
        else [[unlikely]]
        {
            spdlog::error("Invalid grid position: ({}, {}) to add entity {}", pos.x, pos.y, entity);
        }
    }

    void removeStaticEntity(const Tile& pos, uint32_t entity)
    {
        auto layerTypeInt = toInt(MapLayerType::STATIC);
        int maxSize = Constants::MAX_STATIC_ENTITY_TILE_SIZE;
        for (int dx = -maxSize + 1; dx < maxSize; ++dx)
        {
            for (int dy = -maxSize + 1; dy < maxSize; ++dy)
            {
                int nx = pos.x + dx;
                int ny = pos.y + dy;
                if (nx >= 0 && ny >= 0 && nx < static_cast<int>(width) &&
                    ny < static_cast<int>(height))
                {
                    layers[layerTypeInt].cells[nx][ny].removeEntity(entity);
                }
            }
        }
    }

    void removeEntity(MapLayerType layerType, const Tile& pos, uint32_t entity)
    {
        auto layerTypeInt = toInt(layerType);

        if (pos.x >= 0 && pos.y >= 0 && pos.x < width && pos.y < height) [[likely]]
        {
            layers[layerTypeInt].cells[pos.x][pos.y].removeEntity(entity);
        }
        else [[unlikely]]
        {
            spdlog::error("Invalid grid position: ({}, {}) to remove entity {}", pos.x, pos.y,
                          entity);
        }
    }

    void removeAllEntities(MapLayerType layerType, const Tile& pos)
    {
        auto layerTypeInt = toInt(layerType);

        if (pos.x >= 0 && pos.y >= 0 && pos.x < width && pos.y < height) [[likely]]
        {
            layers[layerTypeInt].cells[pos.x][pos.y].removeAllEntities();
        }
        else [[unlikely]]
        {
            spdlog::error("Invalid grid position: ({}, {}) to remove all entities", pos.x, pos.y);
        }
    }

    /**
     * Retrieves the entity ID at the specified grid position and layer.
     *
     * @param layerType The type of map layer to query.
     * @param pos The 2D grid position to look up.
     * @return The entity ID at the given position and layer, or entt::null if the position is
     * invalid.
     *
     * Logs an error if the grid position is out of bounds.
     * Asserts that the layerType is valid.
     */
    uint32_t getEntity(MapLayerType layerType, const Tile& pos) const
    {
        auto layerTypeInt = toInt(layerType);

        if (pos.x >= 0 && pos.y >= 0 && pos.x < width && pos.y < height) [[likely]]
        {
            auto& cell = layers[layerTypeInt].cells[pos.x][pos.y];
            if (cell.isOccupied())
            {
                return cell.getEntity();
            }
            return entt::null;
        }
        else [[unlikely]]
        {
            spdlog::error("Invalid grid position: ({}, {})", pos.x, pos.y);
            return entt::null;
        }
    }

    const std::list<uint32_t>& getEntities(MapLayerType layerType, const Tile& pos) const
    {
        auto layerTypeInt = toInt(layerType);
        static const std::list<uint32_t> empty;

        if (pos.x >= 0 && pos.y >= 0 && pos.x < width && pos.y < height) [[likely]]
        {
            auto& cell = layers[layerTypeInt].cells[pos.x][pos.y];
            return cell.entities;
        }
        else [[unlikely]]
        {
            spdlog::error("Invalid grid position: ({}, {})", pos.x, pos.y);
            return empty;
        }
    }

    bool intersectsStaticObstacle(const Feet& start, const Feet& end) const
    {
        float distance = start.distance(end);
        int numSteps =
            static_cast<int>(distance / (Constants::FEET_PER_TILE * 0.25f)); // Sample every ¼ tile

        if (numSteps <= 0)
            return false;

        Feet step = (end - start) / static_cast<float>(numSteps);

        for (int i = 0; i <= numSteps; ++i)
        {
            Feet point = start + step * static_cast<float>(i);
            Tile tile = point.toTile();

            if (isOccupied(MapLayerType::STATIC, tile))
            {
                return true; // Hit a static obstacle
            }
        }
        return false; // Clear line
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
        int maxLayers = toInt(MapLayerType::MAX_LAYERS) + 1;
        layers = new MapLayer[maxLayers];
        for (size_t i = 0; i < maxLayers; i++)
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