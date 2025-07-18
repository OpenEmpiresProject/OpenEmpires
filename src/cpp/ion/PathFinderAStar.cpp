#include "PathFinderAStar.h"

#include "Tile.h"
#include "utils/Logger.h"

#include <cmath>
#include <limits>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using namespace ion;

double heuristic(const Tile& a, const Tile& b)
{
    // Manhattan distance
    // return std::abs(a.x - b.x) + std::abs(a.y - b.y);

    // Euclidean distance
    return std::sqrt(std::pow(a.x - b.x, 2) + std::pow(a.y - b.y, 2));
}

bool isBlocked(const TileMap& map, const Tile& pos)
{
    if (pos.x < 0 || pos.x >= map.width || pos.y < 0 || pos.y >= map.height ||
        map.isOccupied(MapLayerType::STATIC, pos))
        return true;
    return false;
}

bool isWalkable(const TileMap& map, const Tile& from, const Tile& to)
{
    if (isBlocked(map, to))
        return false;

    if (from.x != to.x && from.y != to.y)
    {
        auto diff = to - from;
        auto corner1 = Tile(from.x, from.y + diff.y);
        auto corner2 = Tile(from.x + diff.x, from.y);

        // Diagonal movement is not allowed if the adjacent tiles are not walkable
        if (isBlocked(map, corner1) || isBlocked(map, corner2))
            return false;
    }
    return true;
}

std::vector<Tile> getNeighbors(const Tile& pos)
{
    return {{pos.x + 1, pos.y},     {pos.x - 1, pos.y},     {pos.x, pos.y + 1},
            {pos.x, pos.y - 1},     {pos.x + 1, pos.y + 1}, {pos.x - 1, pos.y - 1},
            {pos.x + 1, pos.y - 1}, {pos.x - 1, pos.y + 1}};
}

Path reconstructPath(const std::unordered_map<Tile, Tile>& cameFrom, Tile current)
{
    Path path;
    while (cameFrom.contains(current))
    {
        path.push_back(current.centerInFeet());
        current = cameFrom.at(current);
    }
    path.push_back(current.centerInFeet());
    std::reverse(path.begin(), path.end());
    return path;
}

Path PathFinderAStar::findPath(const TileMap& map, const Feet& start, const Feet& goal)
{
    using PQNode = std::pair<int, Tile>; // cost, position
    std::priority_queue<PQNode, std::vector<PQNode>, std::greater<>> open;

    std::unordered_map<Tile, Tile> cameFrom;
    std::unordered_map<Tile, double> gScore;

    auto startTile = start.toTile();
    auto goalTile = goal.toTile();

    open.emplace(0, startTile);
    gScore[startTile] = 0.0;

    Tile closestToGoal = startTile;
    double minHeuristic = heuristic(startTile, goalTile);

    while (!open.empty())
    {
        Tile current = open.top().second;
        open.pop();

        if (current == goalTile)
            return reconstructPath(cameFrom, current);

        double h = heuristic(current, goalTile);
        if (h < minHeuristic)
        {
            minHeuristic = h;
            closestToGoal = current;
        }

        for (const Tile& neighbor : getNeighbors(current))
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
                double fScore = tentativeG + heuristic(neighbor, goalTile);
                open.emplace(fScore, neighbor);
            }
        }
    }

    return reconstructPath(cameFrom, closestToGoal); // Return partial path to nearest point
}
