#include "TileMap.h"

#include "Feet.h"
#include "Tile.h"
#include "logging/Logger.h"
#include "utils/Constants.h"

using namespace core;

bool MapCell::isOccupied() const
{
    return !entities.empty();
}

void MapCell::addEntity(uint32_t entity)
{
    entities.insert(entity);
}

uint32_t MapCell::getEntity() const
{
    return *entities.begin();
}

void MapCell::removeEntity(uint32_t entity)
{
    entities.erase(entity);
}

void MapCell::removeAllEntities()
{
    entities.clear();
}

MapCell** TileMap::getMap(MapLayerType layerType)
{
    return layers[toInt(layerType)].cells;
}

MapCell** TileMap::getStaticMap()
{
    return layers[toInt(MapLayerType::STATIC)].cells;
}

MapCell** TileMap::getGroundMap()
{
    return layers[toInt(MapLayerType::GROUND)].cells;
}

bool TileMap::isOccupied(MapLayerType layerType, const Tile& pos) const
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

bool TileMap::isOccupiedByAnother(MapLayerType layerType, const Tile& pos, uint32_t myEntity) const
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

void TileMap::addEntity(MapLayerType layerType, const Tile& pos, uint32_t entity)
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

void TileMap::addEntity(MapLayerType layerType, const LandArea& landArea, uint32_t entity)
{
    for (const auto& tile : landArea.tiles)
    {
        addEntity(layerType, tile, entity);
    }
}

void TileMap::removeStaticEntity(const Tile& pos, uint32_t entity)
{
    auto layerTypeInt = toInt(MapLayerType::STATIC);
    int maxSize = Constants::MAX_STATIC_ENTITY_TILE_SIZE;
    for (int dx = -maxSize + 1; dx < maxSize; ++dx)
    {
        for (int dy = -maxSize + 1; dy < maxSize; ++dy)
        {
            int nx = pos.x + dx;
            int ny = pos.y + dy;
            if (nx >= 0 && ny >= 0 && nx < static_cast<int>(width) && ny < static_cast<int>(height))
            {
                layers[layerTypeInt].cells[nx][ny].removeEntity(entity);
            }
        }
    }
}

void TileMap::removeEntity(MapLayerType layerType, const Tile& pos, uint32_t entity)
{
    auto layerTypeInt = toInt(layerType);

    if (pos.x >= 0 && pos.y >= 0 && pos.x < width && pos.y < height) [[likely]]
    {
        layers[layerTypeInt].cells[pos.x][pos.y].removeEntity(entity);
    }
    else [[unlikely]]
    {
        spdlog::error("Invalid grid position: ({}, {}) to remove entity {}", pos.x, pos.y, entity);
    }
}

void TileMap::removeEntity(MapLayerType layerType, const LandArea& landArea, uint32_t entity)
{
    for (const auto& tile : landArea.tiles)
    {
        removeEntity(layerType, tile, entity);
    }
}

void TileMap::removeAllEntities(MapLayerType layerType, const Tile& pos)
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

uint32_t TileMap::getEntity(MapLayerType layerType, const Tile& pos) const
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

const std::set<uint32_t>& TileMap::getEntities(MapLayerType layerType, const Tile& pos) const
{
    auto layerTypeInt = toInt(layerType);
    static const std::set<uint32_t> empty;

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

bool TileMap::intersectsStaticObstacle(const Feet& start, const Feet& end) const
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

void TileMap::init(uint32_t width, uint32_t height)
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

bool TileMap::isValidPos(const Tile& pos) const
{
    return pos.x >= 0 && pos.y >= 0 && pos.x < width && pos.y < height;
}
