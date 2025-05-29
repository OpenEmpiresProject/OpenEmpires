#ifndef DRSRESOURCEENTY_H
#define DRSRESOURCEENTY_H

#include <cstdint>

namespace drs
{
    struct DRSResourceEntry 
    {
        int32_t id;
        int32_t offset; // Abosulte
        int32_t size;
    };

    struct DRSResourceData 
    {
        DRSResourceEntry entry;
        uint8_t* data;
    };
} // namespace drs

#endif