#ifndef FOGOFWAR_H
#define FOGOFWAR_H

#include "Feet.h"
#include "Flat2DArray.h"
#include "Tile.h"
#include "utils/Size.h"
#include "utils/Types.h"

#include <cstdint>
#include <vector>

namespace ion
{
class FogOfWar
{
  public:
    void init(uint32_t width, uint32_t height, RevealStatus initialFill);
    void markAsExplored(uint32_t tileX, uint32_t tileY);
    void markAsExplored(const Feet& feetPos);
    void markAsExplored(const Feet& feetPos, uint32_t lineOfSight);
    void markAsExplored(const Feet& feetPos, const Size& size, uint32_t lineOfSight);
    void markAsVisible(uint32_t tileX, uint32_t tileY, uint32_t lineOfSight);
    void markAsVisible(const Feet& feetPos, uint32_t lineOfSight);

    RevealStatus getRevealStatus(uint32_t tileX, uint32_t tileY) const;
    RevealStatus getRevealStatus(const Tile& tilePos) const;
    inline bool isExplored(uint32_t tileX, uint32_t tileY) const
    {
        return m_map.at(tileX, tileY) == RevealStatus::EXPLORED;
    }

    inline bool isExplored(const Tile& tile) const
    {
        return m_map.at(tile.x, tile.y) == RevealStatus::EXPLORED;
    }

    void setRevealMode(uint32_t tileX, uint32_t tileY, RevealStatus type);

  private:
    void markRadius(uint32_t gridX, uint32_t gridY, uint8_t lineOfSight, RevealStatus type);
    void markRadius(const Tile& bottomCorner,
                    const Size& size,
                    uint8_t lineOfSight,
                    RevealStatus type);
    Flat2DArray<RevealStatus> m_map;
};
} // namespace ion

#endif