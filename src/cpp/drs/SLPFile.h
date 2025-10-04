#ifndef SLPFILE_H
#define SLPFILE_H

#include "Frame.h"
#include "FrameInfo.h"

#include <fstream>
#include <vector>

namespace drs
{
class DRSFile;
class SLPFile
{
  public:
    size_t getFrameCount() const;
    Frame getFrame(uint32_t id, uint32_t playerId) const;
    std::vector<Frame> getFrames(uint32_t playerId = 0) const;
    void writeAllFramesToBMP(const std::string& prefix) const;
    uint32_t getId() const
    {
        return m_id;
    }

    std::span<const FrameInfo> getFrameInfos() const
    {
        return m_frameInfos;
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