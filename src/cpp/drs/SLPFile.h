#ifndef SLPFILE_H
#define SLPFILE_H

#include "Frame.h"
#include "DRSResourceEntry.h"

#include <vector>
#include <fstream>

namespace drs
{
    class SLPFile
    {
    public:
        SLPFile(const DRSResourceData& resource);
        size_t getFrameCount() const;
        Frame getFrame(size_t index) const;
        std::vector<Frame> getFrames() const;

    private:
        const DRSResourceData& m_resourceData;
        std::span<const SLPFrameInfo> m_frameInfos;

        void init();
    };

}
#endif