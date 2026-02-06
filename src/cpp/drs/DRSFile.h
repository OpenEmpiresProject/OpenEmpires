#ifndef DRSFILE_H
#define DRSFILE_H

#include "SLPFile.h"

#include <memory>
#include <mutex>
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
    SLPFile getSLPFile(uint32_t resourceId);
    std::vector<uint32_t> listResources() const;
    const std::string& getFilename() const
    {
        return m_filename;
    }

  private:
    std::shared_ptr<DRSResourceData> getResource(uint32_t resourceId);

    std::ifstream m_file;
    std::unordered_map<uint32_t, std::shared_ptr<DRSResourceData>> m_resources;
    std::mutex m_fileMutex;
    std::string m_filename;
};
} // namespace drs

#endif