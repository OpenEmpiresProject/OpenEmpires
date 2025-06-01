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
    Frame getFrame(uint32_t id) const;
    std::vector<Frame> getFrames() const;
    void writeAllFramesToBMP(const std::string& prefix) const;
    uint32_t getId() const
    {
        return m_id;
    }

  private:
    friend class DRSFile;

    SLPFile(uint32_t id, const std::span<const uint8_t>& slpData);
    void init();

    const std::span<const uint8_t> m_slpData;
    std::span<const FrameInfo> m_frameInfos;
    uint32_t m_id = 0;
};

} // namespace drs
#endif