#ifndef GAME_DRSINTERFACE_H
#define GAME_DRSINTERFACE_H
#include "DRSFile.h"
#include "Rect.h"
#include "utils/Types.h"

namespace game
{
class DRSInterface
{
  public:
    DRSInterface();
    virtual ~DRSInterface();

    virtual core::Ref<drs::DRSFile> loadDRSFile(const std::string& filename);
    virtual core::Rect<int> getBoundingBox(const std::string& drsFilename,
                                           int slpId,
                                           int frameIndex = 0);

  private:
    std::unordered_map<std::string, core::Ref<drs::DRSFile>> m_drsFilesByName;
};
} // namespace game

#endif // GAME_DRSINTERFACE_H
