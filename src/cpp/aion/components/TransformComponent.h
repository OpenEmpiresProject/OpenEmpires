#ifndef TRANSFORMCOMPONENT_H
#define TRANSFORMCOMPONENT_H

#include "Component.h"
#include "Vec2d.h"
#include "Types.h"
#include <numbers>

namespace aion
{
    class TransformComponent : public aion::Component<TransformComponent>
    {
    public:
        aion::Vec2d position{0, 0}; // Position in the game world
        int rotation = 0; // Rotation in degrees from North (0 degrees is up)

        TransformComponent() = default;
        TransformComponent(int x, int y) : position(x, y) {}
        TransformComponent(const aion::Vec2d& pos) : position(pos) {}
        TransformComponent(const TransformComponent&) = default;
        TransformComponent& operator=(const TransformComponent&) = default;

        void face(const aion::Vec2d& target)
        {
            // Calculate the angle to face the target position
            int deltaX = target.x - position.x;
            int deltaY = target.y - position.y;
            rotation = static_cast<int>(std::atan2(deltaY, deltaX) * 180 / std::numbers::pi);
        }

        void face(utils::Direction direction)
        {
            rotation = 45 * static_cast<int>(direction);
        }

        utils::Direction getDirection() const
        {
            return static_cast<utils::Direction>((rotation % 360) / 45);
        }
    };
}


#endif