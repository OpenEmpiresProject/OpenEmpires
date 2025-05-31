#ifndef DRSFILE_H
#define DRSFILE_H

#include "DRSResourceEntry.h"
#include "SLPFile.h"

#include <cstdint>
#include <fstream>
#include <span>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

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