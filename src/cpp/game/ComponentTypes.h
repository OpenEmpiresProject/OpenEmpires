#ifndef COMPONENTTYPES_H
#define COMPONENTTYPES_H

namespace game
{
    enum class ComponentType
    {
        NONE = 0,
        PHYSICS, // eg: Position, Velocity
        GRAPHICS // eg: Sprite, Animation
    };
}

#endif