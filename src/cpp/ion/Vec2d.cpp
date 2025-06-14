#include "Vec2d.h"

#include <limits>

const ion::Vec2d ion::Vec2d::null =
    ion::Vec2d(std::numeric_limits<int>::min(), std::numeric_limits<int>::min());
