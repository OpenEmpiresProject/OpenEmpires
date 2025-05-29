#ifndef DRSFILE_H
#define DRSFILE_H

#include "SLPFile.h"
#include "DRSResourceEntry.h"

#include <fstream>
#include <unordered_map>
#include <vector>
#include <cstdint>
#include <string>
#include <stdexcept>
#include <span>

namespace drs
{
    class DRSFile 
    {
    public:
        bool load(const std::string& filename);
        DRSResourceData getResource(int resourceId);
        SLPFile getSLPFile(int resourceId);
        std::vector<int> listResources() const;

    private:
        std::ifstream file;
        std::unordered_map<int, DRSResourceEntry> m_resources;
    };
} // namespace drs


#endif