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

  private:
    friend class SLPFile;

    Frame(const FrameInfo& fi); // Will be constructed only by SLPFile
    void load(std::span<const uint8_t> slpData);
    void writeToBMP(const std::string& filename) const;

    std::vector<std::vector<Color>> m_image;
    const FrameInfo& m_frameInfo;

    static inline const int MAX_IMAGE_SIZE = 500;
};
} // namespace drs

#endif