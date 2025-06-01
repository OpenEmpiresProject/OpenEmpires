#ifndef DRSTESTER_H
#define DRSTESTER_H

#include "DRSFile.h"

#include <iostream>

namespace drs
{
class DRSTester
{
  public:
    void load()
    {
        DRSFile drs;
        if (drs.load("terrain.drs"))
        {
            auto ids = drs.listResources();
            for (uint32_t id : ids)
            {
                // auto data = drs.getResource(id);
                std::cout << "Loaded resource " << id << "\n";
                // std::cout << "Loaded resource " << id << " (" << data.size() << " bytes)\n";
            }

            auto slp = drs.getSLPFile(15001);
            // // auto frame = slp.getFrame(0);
            // // frame.writeToBMP("test.bmp");
            slp.writeAllFramesToBMP("15001_");
        }
    }
};
} // namespace drs

#endif