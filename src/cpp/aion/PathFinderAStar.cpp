#include "PathFinderAStar.h"

#include <cmath>
#include <limits>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace aion;

Path PathFinderAStar::findPath(const StaticEntityMap& map, const Vec2d& start, const Vec2d& goal)
{
    using PQNode = std::pair<int, Vec2d>; // cost, position
    std::priority_queue<PQNode, std::vector<PQNode>, std::greater<>> open;

    std::unordered_map<Vec2d, Vec2d> cameFrom;
    std::unordered_map<Vec2d, int> gScore;

    open.emplace(0, start);
    gScore[start] = 0;

    while (!open.empty())
    {
        Vec2d current = open.top().second;
        open.pop();

        if (current == goal)
            return reconstructPath(cameFrom, current);

        for (const Vec2d& neighbor : getNeighbors(current))
        {
            if (!isWalkable(map, neighbor))
                continue;

            int tentativeG = gScore[current] + 1;

            if (!gScore.contains(neighbor) || tentativeG < gScore[neighbor])
            {
                cameFrom[neighbor] = current;
                gScore[neighbor] = tentativeG;
                int fScore = tentativeG + heuristic(neighbor, goal);
                open.emplace(fScore, neighbor);
            }
        }
    }

    return {}; // No path found
}

int PathFinderAStar::heuristic(const Vec2d& a, const Vec2d& b)
{
    // Manhattan distance
    return std::abs(a.x - b.x) + std::abs(a.y - b.y);
}

bool PathFinderAStar::isWalkable(const StaticEntityMap& map, const Vec2d& pos)
{
    return pos.x >= 0 && pos.x < map.width && pos.y >= 0 && pos.y < map.height &&
           map.map[pos.y][pos.x] == 0;
}

std::vector<Vec2d> PathFinderAStar::getNeighbors(const Vec2d& pos)
{
    return {
        {pos.x + 1, pos.y},
        {pos.x - 1, pos.y},
        {pos.x, pos.y + 1},
        {pos.x, pos.y - 1},
    };
}

Path PathFinderAStar::reconstructPath(const std::unordered_map<Vec2d, Vec2d>& cameFrom,
                                      Vec2d current)
{
    Path path;
    while (cameFrom.contains(current))
    {
        path.push_back(current);
        current = cameFrom.at(current);
    }
    path.push_back(current);
    std::reverse(path.begin(), path.end());
    return path;
}
