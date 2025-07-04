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
    // Engine will lookup MAX_SELECTION_LOOKUP_HEIGHT*MAX_SELECTION_LOOKUP_HEIGHT area
    // with reverse Zordering to find the object with clicked position is within the
    //  object's image boundaries
    static const int MAX_SELECTION_LOOKUP_HEIGHT = 4;
    static const int MAX_RESOURCE_TYPES = 8;
    static const int RESOURCE_TYPE_NONE = 0;
    // The villager will look MAX_RESOURCE_LOOKUP_RADIUS*MAX_RESOURCE_LOOKUP_RADIUS square for
    // similar resource once the current resource exhausted
    static const int MAX_RESOURCE_LOOKUP_RADIUS = 4;
    // A static entity such as building can occupy at most 4x4 tiles
    static const int MAX_STATIC_ENTITY_TILE_SIZE = 4;
    static const int FIXED_FPS = 60; // Frames per second
    // Maximum gap between two frames. If the gap is more than this,
    // frame delay will be capped to this.
    static const int MAX_FRAME_DELAY_MS = 500;
};
} // namespace ion

#endif