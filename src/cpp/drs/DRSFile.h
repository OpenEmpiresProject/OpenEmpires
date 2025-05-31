#ifndef DRSFILE_H
#define DRSFILE_H

#include "SLPFile.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace drs
{
class DRSResourceData;
class DRSFile
{
  public:
    bool load(const std::string& filename);
    SLPFile getSLPFile(int resourceId);
    std::vector<int> listResources() const;

  private:
    std::shared_ptr<DRSResourceData> getResource(int resourceId);

    std::ifstream m_file;
    std::unordered_map<int, std::shared_ptr<DRSResourceData>> m_resources;
};
} // namespace drs

#endif