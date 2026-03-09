#ifndef CORE_PATH_H
#define CORE_PATH_H

#include "Feet.h"

#include <list>
#include <vector>

namespace core
{
class PathService;
class Path
{
  public:
    Path();
    Path(const std::vector<Feet>& waypoints);
    ~Path();
    std::list<Feet>& getWaypoints();
    const std::list<Feet>& getWaypoints() const;

  private:
    friend class PathService;
    std::list<Feet> waypoints;
};
} // namespace core

#endif // CORE_PATH_H
