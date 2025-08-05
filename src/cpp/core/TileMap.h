#ifndef TILEMAP_H
#define TILEMAP_H

#include "debug.h"
#include "utils/Types.h"

#include <algorithm>
#include <entt/entity/registry.hpp>
#include <list>

namespace core
{
class Tile;
class Feet;

struct MapCell
{
    std::list<uint32_t> entities; // List of entities occupying this cell

    bool isOccupied() const;
    void addEntity(uint32_t entity);
    uint32_t getEntity() const;
    void removeEntity(uint32_t entity);
    void removeAllEntities();
};

struct MapLayer
{
    MapCell** cells = nullptr;
};

enum class MapLayerType
{
    GROUND = 0,
    ON_GROUND, // walkable decorators/items on the ground
    STATIC,    // doesn't move, not walkable
    UNITS,
    // Add more layers here

    MAX_LAYERS
};

struct TileMap
{
    uint32_t width = 0;
    uint32_t height = 0;
    MapLayer* layers = nullptr;

    MapCell** getMap(MapLayerType layerType);
    MapCell** getStaticMap();
    MapCell** getGroundMap();

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
    bool isOccupied(MapLayerType layerType, const Tile& pos) const;

    bool isOccupiedByAnother(MapLayerType layerType, const Tile& pos, uint32_t myEntity) const;

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
    void addEntity(MapLayerType layerType, const Tile& pos, uint32_t entity);

    void removeStaticEntity(const Tile& pos, uint32_t entity);
    void removeEntity(MapLayerType layerType, const Tile& pos, uint32_t entity);
    void removeAllEntities(MapLayerType layerType, const Tile& pos);

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
    uint32_t getEntity(MapLayerType layerType, const Tile& pos) const;

    const std::list<uint32_t>& getEntities(MapLayerType layerType, const Tile& pos) const;
    bool intersectsStaticObstacle(const Feet& start, const Feet& end) const;

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
    void init(uint32_t width, uint32_t height);
};

} // namespace core

#endif