#ifndef FOGOFWAR_H
#define FOGOFWAR_H

#include "Flat2DArray.h"
#include "utils/Types.h"

#include <cstdint>
#include <vector>

namespace core
{
class Feet;
class Tile;
class Size;

class FogOfWar
{
  public:
    void init(uint32_t width, uint32_t height, RevealStatus initialFill);
    void markAsExplored(const Tile& tilePos);
    void markAsExplored(const Feet& feetPos);
    void markAsExplored(const Feet& feetPos, uint32_t lineOfSight);
    void markAsExplored(const Feet& feetPos, const Size& size, uint32_t lineOfSight);
    void markAsVisible(const Tile& tilePos, uint32_t lineOfSight);
    void markAsVisible(const Feet& feetPos, uint32_t lineOfSight);

    void setRevealStatus(const Tile& tilePos, RevealStatus type);
    RevealStatus getRevealStatus(const Tile& tilePos) const;
    bool isExplored(const Tile& tile) const;

  private:
    void markRadius(uint32_t tileX, uint32_t tileY, uint8_t lineOfSight, RevealStatus type);
    void markRadius(const Tile& bottomCorner,
                    const Size& size,
                    uint8_t lineOfSight,
                    RevealStatus type);

  private:
    Flat2DArray<RevealStatus> m_map;
};
} // namespace core

#endif