#ifndef COMPRESOURCE_H
#define COMPRESOURCE_H

#include "Resource.h"
#include "utils/Constants.h"

#include <cstdint>

namespace ion
{
struct CompResource
{
    Resource resource;
    uint32_t originalAmount = 0;

    CompResource(const Resource& resource) : resource(resource), originalAmount(resource.amount)
    {
    }
};
} // namespace ion

#endif