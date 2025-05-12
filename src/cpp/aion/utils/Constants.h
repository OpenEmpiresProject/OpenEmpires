#ifndef CONSTANTS_H
#define CONSTANTS_H

namespace aion
{
class Constants
{
  public:
    static const int MAX_ENTITIES = 1000000;
    static const int MAX_COMPONENTS = 64; // Maximum number of components per entity
    static const int FEET_PER_TILE = 256; // Size of each tile in game unit of distance measurement
    static const int TILE_PIXEL_WIDTH = 96;
    static const int TILE_PIXEL_HEIGHT = 48;
    static const int MAX_ANIMATIONS = 10;
};
} // namespace aion

#endif