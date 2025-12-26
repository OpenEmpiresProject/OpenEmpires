#include "DRSInterface.h"

using namespace game;

DRSInterface::DRSInterface()
{
    // constructor
}

DRSInterface::~DRSInterface()
{
    // destructor
}

core::Ref<drs::DRSFile> DRSInterface::loadDRSFile(const std::string& filename)
{
    auto it = m_drsFilesByName.find(filename);
    if (it != m_drsFilesByName.end())
        return it->second;
    else
    {
        auto drs = std::make_shared<drs::DRSFile>();
        if (!drs->load(filename))
        {
            throw std::runtime_error("Failed to load DRS file: " + filename);
        }
        return drs;
    }
}

core::Rect<int> DRSInterface::getBoundingBox(const std::string& drsFilename,
                                             int slpId,
                                             int frameIndex /*= 0*/)
{
    auto drsFile = loadDRSFile(drsFilename);
    auto frameInfos = drsFile->getSLPFile(slpId).getFrameInfos();
    auto frame = frameInfos[frameIndex];
    core::Rect<int> box(frame.hotspot_x, frame.hotspot_y, frame.width, frame.height);
    return box;
}
