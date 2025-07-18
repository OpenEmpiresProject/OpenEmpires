#ifndef UNITSELECTION_H
#define UNITSELECTION_H

#include "utils/Constants.h"

#include <vector>

namespace ion
{
struct UnitSelection
{
    std::vector<uint32_t> selectedEntities;

    UnitSelection()
    {
        selectedEntities.reserve(Constants::MAX_UNIT_SELECTION);
    }
    ~UnitSelection() = default;
};
} // namespace ion

#endif