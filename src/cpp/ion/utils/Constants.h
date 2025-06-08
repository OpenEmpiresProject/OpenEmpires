#ifndef CONSTANTS_H
#define CONSTANTS_H

namespace ion
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
    static const int MAX_UNIT_SELECTION = 30;
    // Mouse should move at least 5 pixels to consider as a selection box
    static const int MIN_SELECTION_BOX_MOUSE_MOVEMENT = 25;
    // Engine will lookup MAX_SELECTION_LOOKUP_HEIGHTxMAX_SELECTION_LOOKUP_HEIGHT area
    // with reverse Zordering to find the object with clicked position is within the
    //  object's image boundaries
    static const int MAX_SELECTION_LOOKUP_HEIGHT = 4;
};
} // namespace ion

#endif