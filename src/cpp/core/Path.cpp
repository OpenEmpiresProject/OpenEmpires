#include "Path.h"

using namespace core;

Path::Path(const std::vector<Feet>& _waypoints) : waypoints(_waypoints.begin(), _waypoints.end())
{
}

Path::Path()
{
}

Path::~Path()
{
    // destructor
}

const std::list<core::Feet>& Path::getWaypoints() const
{
    return waypoints;
}

std::list<core::Feet>& Path::getWaypoints()
{
    return waypoints;
}

bool Path::isEmpty() const
{
    return waypoints.empty();
}

const core::Feet& Path::nextWaypoint() const
{
    if (waypoints.empty())
    {
        return Feet::null;
    }
    return waypoints.front();
}

void Path::removeNextWaypoint()
{
    if (!waypoints.empty())
    {
        waypoints.pop_front();
    }
}
