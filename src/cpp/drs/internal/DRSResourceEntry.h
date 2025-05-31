#ifndef DRSRESOURCEENTY_H
#define DRSRESOURCEENTY_H

#include <cstdint>

namespace drs
{
struct DRSResourceEntry
{
    int32_t id = 0;
    int32_t offset = 0; // Abosulte
    int32_t size = 0;
};

struct DRSResourceData
{
    DRSResourceEntry entry;
    uint8_t* data = nullptr;

    ~DRSResourceData()
    {
        if (data != nullptr)
            delete[] data;
    }
};
} // namespace drs

#endif