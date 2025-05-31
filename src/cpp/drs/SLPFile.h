#ifndef SLPFILE_H
#define SLPFILE_H

#include "DRSResourceEntry.h"
#include "Frame.h"

#include <fstream>
#include <vector>

namespace drs
{
class SLPFile
{
  public:
    SLPFile(const DRSResourceData& resource);
    size_t getFrameCount() const;
    Frame getFrame(size_t index) const;
    std::vector<Frame> getFrames() const;
    void writeAllFramesToBMP(const std::string& prefix) const;

  private:
    const DRSResourceData m_resourceData;
    std::span<const SLPFrameInfo> m_frameInfos;

    void init();
};

} // namespace drs
#endif