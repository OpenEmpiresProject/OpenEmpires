#ifndef CONSTANTS_H
#define CONSTANTS_H

namespace utils
{
    class Constants
    {
    public:
        static const int MAX_ENTITIES = 1000000;
        static const int MAX_COMPONENTS = 64; // Maximum number of components per entity
        static const int TILE_SIZE = 256; // Size of each tile in game unit of distance measurement
    };
}

#endif