#ifndef SLPFILE_H
#define SLPFILE_H

#include "Frame.h"

#include <fstream>
#include <vector>

namespace drs
{
class FrameInfo;
class DRSFile;
class SLPFile
{
  public:
    size_t getFrameCount() const;
    Frame getFrame(size_t index) const;
    std::vector<Frame> getFrames() const;
    void writeAllFramesToBMP(const std::string& prefix) const;

  private:
    friend class DRSFile;

    SLPFile(const std::span<const uint8_t>& slpData);
    void init();

    const std::span<const uint8_t> m_slpData;
    std::span<const FrameInfo> m_frameInfos;
};

} // namespace drs
#endif