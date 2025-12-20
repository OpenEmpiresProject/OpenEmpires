#include "Utils.h"

using namespace core;

/*
 *   Behavior:
 *       1. The path always consists of at most one 90 turn
 *       2. The corner orientation (L-shape) depends on drag direction:
 *           - If you drag more horizontally, the path goes horizontal first, then vertical.
 *           - If you drag more vertically, it goes vertical first, then horizontal.
 *       3. If the horizontal and vertical drags are same, then path is diagonal
 */
void Utils::calculateConnectedBuildingsPath(const Tile& start,
                                            const Tile& end,
                                            std::list<TilePosWithOrientation>& connectedBuildings)
{
    const int dx = std::abs(start.x - end.x);
    const int dy = std::abs(start.y - end.y);

    Tile corner = Tile::null;

    if (dx > dy)
    {
        corner = Tile(end.x, start.y);

        uint32_t x = std::min(start.x, end.x) + 1;
        const uint32_t maxX = std::max(start.x, end.x);

        for (; x < maxX; ++x)
        {
            Tile newPos(x, start.y);
            connectedBuildings.push_back(
                TilePosWithOrientation{newPos, BuildingOrientation::DIAGONAL_BACKWARD});
        }

        uint32_t y = std::min(start.y, end.y) + 1;
        const uint32_t maxY = std::max(start.y, end.y);

        for (; y < maxY; ++y)
        {
            Tile newPos(end.x, y);
            connectedBuildings.push_back(
                TilePosWithOrientation{newPos, BuildingOrientation::DIAGONAL_FORWARD});
        }
    }
    else if (dy > dx)
    {
        corner = Tile(start.x, end.y);

        uint32_t y = std::min(start.y, end.y) + 1;
        const uint32_t maxY = std::max(start.y, end.y);

        for (; y < maxY; ++y)
        {
            Tile newPos(start.x, y);
            connectedBuildings.push_back(
                TilePosWithOrientation{newPos, BuildingOrientation::DIAGONAL_FORWARD});
        }

        uint32_t x = std::min(start.x, end.x) + 1;
        const uint32_t maxX = std::max(start.x, end.x);

        for (; x < maxX; ++x)
        {
            Tile newPos(x, end.y);
            connectedBuildings.push_back(
                TilePosWithOrientation{newPos, BuildingOrientation::DIAGONAL_BACKWARD});
        }
    }
    else // dx == dy. Visual horizontal/vertical, logical diagonal
    {
        const int xDirection = (end.x - start.x) / (dx == 0 ? 1 : dx);
        const int yDirection = (end.y - start.y) / (dy == 0 ? 1 : dy);

        const int angle = xDirection * yDirection;

        // Moving in both direction by 1 tile. i.e. skipping start building since it will be
        // added later anyway
        int newX = start.x + xDirection;
        int newY = start.y + yDirection;

        while (newX != end.x and newY != end.y)
        {
            Tile newPos(newX, newY);

            connectedBuildings.push_back(TilePosWithOrientation{
                newPos,
                (angle >= 0 ? BuildingOrientation::VERTICAL : BuildingOrientation::HORIZONTAL)});

            newX += xDirection;
            newY += yDirection;
        }
    }

    connectedBuildings.push_back(TilePosWithOrientation{start, BuildingOrientation::CORNER});

    if (start != end)
        connectedBuildings.push_back(TilePosWithOrientation{end, BuildingOrientation::CORNER});

    if (start != corner and end != corner and corner.isNull() == false)
        connectedBuildings.push_back(TilePosWithOrientation{corner, BuildingOrientation::CORNER});
}
