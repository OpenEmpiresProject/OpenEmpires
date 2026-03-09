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
