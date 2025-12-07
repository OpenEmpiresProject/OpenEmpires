#ifndef CORE_TILEMAPLISTNER_H
#define CORE_TILEMAPLISTNER_H

#include "Tile.h"
#include "utils/Types.h"

namespace core
{
class TileMapListner
{
  public:
    virtual void onEntityEnter(uint32_t entity, const Tile& tile, MapLayerType layer) {};
    virtual void onEntityExit(uint32_t entity, const Tile& tile, MapLayerType layer) {};
};
} // namespace core

#endif // CORE_TILEMAPLISTNER_H
