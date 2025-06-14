#ifndef FRAME_H
#define FRAME_H

#include "Color.h"

#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace drs
{
class FrameInfo;
class SLPFile;
class Frame
{
  public:
    const std::vector<std::vector<Color>>& getImage() const;
    std::pair<int, int> getAnchor() const;
    std::pair<int, int> getDimensions() const;
    uint32_t getId() const
    {
        return m_id;
    }
    const std::string& getFQID() const;

  private:
    friend class SLPFile;

    Frame(uint32_t parentId,
          uint32_t id,
          const FrameInfo& fi); // Will be constructed only by SLPFile
    void load(std::span<const uint8_t> slpData);
    void writeToBMP(const std::string& filename) const;

    std::vector<std::vector<Color>> m_image;
    const FrameInfo& m_frameInfo;
    const uint32_t m_id = 0;
    const uint32_t m_parentId = 0;
    const std::string m_fqid;

    static inline const int MAX_IMAGE_SIZE = 2000;
};
} // namespace drs

#endif