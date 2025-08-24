#ifndef UNITSELECTION_H
#define UNITSELECTION_H

#include "utils/Constants.h"

#include <vector>

namespace core
{
struct EntitySelection
{
    std::vector<uint32_t> selectedEntities;

    EntitySelection()
    {
        selectedEntities.reserve(Constants::MAX_ENTITY_SELECTION);
    }
};
} // namespace core

#endif