#include "PathFinderAStar.h"

#include "utils/Logger.h"

#include <cmath>
#include <limits>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace ion;

double heuristic(const Vec2d& a, const Vec2d& b)
{
    // Manhattan distance
    // return std::abs(a.x - b.x) + std::abs(a.y - b.y);

    // Euclidean distance
    return std::sqrt(std::pow(a.x - b.x, 2) + std::pow(a.y - b.y, 2));
}

bool isBlocked(const GridMap& map, const Vec2d& pos)
{
    if (pos.x < 0 || pos.x >= map.width || pos.y < 0 || pos.y >= map.height ||
        map.isOccupied(MapLayerType::STATIC, pos))
        return true;
    return false;
}

bool isWalkable(const GridMap& map, const Vec2d& from, const Vec2d& to)
{
    if (isBlocked(map, to))
        return false;

    if (from.x != to.x && from.y != to.y)
    {
        auto diff = to - from;
        auto corner1 = Vec2d(from.x, from.y + diff.y);
        auto corner2 = Vec2d(from.x + diff.x, from.y);

        // Diagonal movement is not allowed if the adjacent tiles are not walkable
        if (isBlocked(map, corner1) || isBlocked(map, corner2))
            return false;
    }
    return true;
}

std::vector<Vec2d> getNeighbors(const Vec2d& pos)
{
    return {{pos.x + 1, pos.y},     {pos.x - 1, pos.y},     {pos.x, pos.y + 1},
            {pos.x, pos.y - 1},     {pos.x + 1, pos.y + 1}, {pos.x - 1, pos.y - 1},
            {pos.x + 1, pos.y - 1}, {pos.x - 1, pos.y + 1}};
}

Path reconstructPath(const std::unordered_map<Vec2d, Vec2d>& cameFrom, Vec2d current)
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

Path PathFinderAStar::findPath(const GridMap& map, const Vec2d& start, const Vec2d& goal)
{
    using PQNode = std::pair<int, Vec2d>; // cost, position
    std::priority_queue<PQNode, std::vector<PQNode>, std::greater<>> open;

    std::unordered_map<Vec2d, Vec2d> cameFrom;
    std::unordered_map<Vec2d, double> gScore;

    open.emplace(0, start);
    gScore[start] = 0.0;

    Vec2d closestToGoal = start;
    double minHeuristic = heuristic(start, goal);

    while (!open.empty())
    {
        Vec2d current = open.top().second;
        open.pop();

        if (current == goal)
            return reconstructPath(cameFrom, current);

        double h = heuristic(current, goal);
        if (h < minHeuristic)
        {
            minHeuristic = h;
            closestToGoal = current;
        }

        for (const Vec2d& neighbor : getNeighbors(current))
        {
            if (!isWalkable(map, current, neighbor))
            {
                // spdlog::error("Neighbor {} is blocked", neighbor.toString());
                continue;
            }

            double moveCost = (current.x != neighbor.x && current.y != neighbor.y) ? 1.4 : 1.0;
            double tentativeG = gScore[current] + moveCost;

            if (!gScore.contains(neighbor) || tentativeG < gScore[neighbor])
            {
                cameFrom[neighbor] = current;
                gScore[neighbor] = tentativeG;
                double fScore = tentativeG + heuristic(neighbor, goal);
                open.emplace(fScore, neighbor);
            }
        }
    }

    return reconstructPath(cameFrom, closestToGoal); // Return partial path to nearest point
}
