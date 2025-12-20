#ifndef GAMESETTINGS_H
#define GAMESETTINGS_H

#include "utils/Constants.h"
#include "utils/Size.h"
#include "utils/Types.h"

#include <stdexcept>

namespace core
{
class Settings
{
  public:
    void setResolution(int width, int height);
    void setFullscreen(bool fullscreen);
    void setVSync(bool vsync);
    void setVolume(float volume);
    void setMusicVolume(float volume);
    void setWindowDimensions(int width, int height);
    void setWorldSizeType(WorldSizeType size);
    void setTitle(const std::string& m_title);
    void setViewportMovingSpeed(int speed);
    void setTicksPerSecond(int tps);
    void setTargetFPS(int fps);
    const Size& getResolution() const;
    const Size& getWindowDimensions() const;
    WorldSizeType getWorldSizeType() const;
    Size getWorldSize() const;
    Size getWorldSizeInTiles() const;
    bool isFullscreen() const;
    bool isVSync() const;
    float getVolume() const;
    float getMusicVolume() const;
    std::string getTitle() const;
    int getViewportMovingSpeed() const;
    int getTicksPerSecond() const;
    int getTargetFPS() const;
    RevealStatus getFOWRevealStatus() const;
    void setFOWRevealStatus(RevealStatus mode);
    uint32_t getMaxPopulation() const;
    float getGameSpeed() const;
    void setGameSpeed(float val);

  private:
    Size m_resolution{800, 600};
    Size m_windowDimensions{800, 600};
    WorldSizeType m_worldSizeType = WorldSizeType::DEMO;
    bool m_isFullscreen = false;
    bool m_isVSync = true;
    float m_soundVolume = 1.0f;
    float m_musicVolume = 1.0f;
    std::string m_title = "openEmipires";
    int m_viewportMovingSpeed = 100;
    int m_ticksPerSecond = 60;
    int m_targetFPS = 60;
    RevealStatus m_fowTRevealStatus = RevealStatus::UNEXPLORED;
    uint32_t m_maxPopulation = 10;
    float m_gameSpeed = 1.0;
};
} // namespace core

#endif