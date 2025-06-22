#ifndef FOGOFWAR_H
#define FOGOFWAR_H

#include "Coordinates.h"
#include "Feet.h"
#include "Tile.h"
#include "utils/Types.h"

#include <cstdint>
#include <vector>

namespace ion
{
class FogOfWar
{
  public:
    FogOfWar(/* args */);
    ~FogOfWar();

    void init(uint32_t width, uint32_t height, RevealStatus initialFill);
    void markAsExplored(uint32_t tileX, uint32_t tileY);
    void markAsExplored(const Feet& feetPos);
    void markAsExplored(const Feet& feetPos, uint32_t lineOfSight);
    void markAsExplored(const Feet& feetPos, const Size& size, uint32_t lineOfSight);
    void markAsVisible(uint32_t tileX, uint32_t tileY, uint32_t lineOfSight);
    void markAsVisible(const Feet& feetPos, uint32_t lineOfSight);

    const std::vector<std::vector<RevealStatus>>& getMap() const;
    RevealStatus getRevealMode(uint32_t tileX, uint32_t tileY) const;
    RevealStatus getRevealMode(const Tile& tilePos) const;
    void setRevealMode(uint32_t tileX, uint32_t tileY, RevealStatus type, bool isEdge = false);
    void disable();
    void enable();

  private:
    void markRadius(uint32_t gridX, uint32_t gridY, uint8_t lineOfSight, RevealStatus type);
    void markRadius(const Tile& bottomCorner, const Size& size, uint8_t lineOfSight, RevealStatus type);
    uint32_t m_width = 0;
    uint32_t m_height = 0;
    std::vector<std::vector<RevealStatus>> m_map;
    Ref<Coordinates> m_coordinates;
};
} // namespace ion

#endif