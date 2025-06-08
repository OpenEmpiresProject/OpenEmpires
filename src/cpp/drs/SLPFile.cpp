#include "SLPFile.h"

#include "DRSFile.h"
#include "FrameInfo.h"

#include <iostream>
#include <string>

using namespace drs;

#pragma pack(push, 1)
struct SLPHeader
{
    char version[4];
    int32_t numOfFrames;
    char comment[24];
};

#pragma pack(pop)

SLPFile::SLPFile(uint32_t id, const std::span<const uint8_t>& slpData)
    : m_slpData(slpData), m_id(id)
{
    init();
}

size_t SLPFile::getFrameCount() const
{
    return m_frameInfos.size();
}

Frame SLPFile::getFrame(uint32_t id) const
{
    const FrameInfo& fi = m_frameInfos[id - 1];
    Frame frame(m_id, id, fi);
    frame.load(m_slpData);
    return frame;
}

std::vector<Frame> SLPFile::getFrames() const
{
    std::vector<Frame> frames;

    for (int i = 1; i <= getFrameCount(); ++i)
    {
        frames.push_back(getFrame(i));
    }
    return frames;
}

void SLPFile::writeAllFramesToBMP(const std::string& prefix) const
{
    auto frames = getFrames();
    for (size_t i = 0; i < frames.size(); i++)
    {
        frames[i].writeToBMP(prefix + std::to_string(i) + ".bmp");
    }
}

void SLPFile::init()
{
    const uint8_t* data = m_slpData.data();
    size_t size = m_slpData.size();

    if (size < sizeof(SLPHeader))
    {
        throw std::runtime_error("SLP file too small for header.");
    }

    // Read SLPHeader
    const SLPHeader* header = reinterpret_cast<const SLPHeader*>(data);

    // Verify version if needed (e.g., "2.0N")
    if (std::string(header->version, 4) != "2.0N")
    {
        throw std::runtime_error("Unsupported SLP version.");
    }

    int32_t frameCount = header->numOfFrames;
    const size_t frameInfoSize = sizeof(FrameInfo);
    const size_t frameInfosOffset = sizeof(SLPHeader);

    // Bounds check
    if (size < frameInfosOffset + frameCount * frameInfoSize)
    {
        throw std::runtime_error("SLP file too small for frame infos.");
    }

    // Load frame info table
    const FrameInfo* frameInfos = reinterpret_cast<const FrameInfo*>(data + frameInfosOffset);
    m_frameInfos = std::span<const FrameInfo>(frameInfos, frameCount);
}
