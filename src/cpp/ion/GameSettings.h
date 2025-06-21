#ifndef GAMESETTINGS_H
#define GAMESETTINGS_H

#include "utils/Constants.h"
#include "utils/Size.h"
#include "utils/Types.h"

namespace ion
{
class GameSettings
{
  public:
    GameSettings() = default;
    ~GameSettings() = default;

    void setResolution(int width, int height)
    {
        m_resolution.width = width;
        m_resolution.height = height;
    }

    void setFullscreen(bool fullscreen)
    {
        m_isFullscreen = fullscreen;
    }

    void setVSync(bool vsync)
    {
        m_isVSync = vsync;
    }

    void setVolume(float volume)
    {
        m_soundVolume = volume;
    }

    void setMusicVolume(float volume)
    {
        m_musicVolume = volume;
    }

    void setWindowDimensions(int width, int height)
    {
        m_windowDimensions.width = width;
        m_windowDimensions.height = height;
    }

    void setWorldSizeType(WorldSizeType size)
    {
        m_worldSizeType = size;
    }

    void setTitle(const std::string& m_title)
    {
        this->m_title = m_title;
    }

    void setViewportMovingSpeed(int speed)
    {
        m_viewportMovingSpeed = speed;
    }

    void setTicksPerSecond(int tps)
    {
        m_ticksPerSecond = tps;
    }

    void setTargetFPS(int fps)
    {
        m_targetFPS = fps;
    }

    const Size& getResolution() const
    {
        return m_resolution;
    }

    const Size& getWindowDimensions() const
    {
        return m_windowDimensions;
    }

    WorldSizeType getWorldSizeType() const
    {
        return m_worldSizeType;
    }

    Size getWorldSize() const
    {
        switch (m_worldSizeType)
        {
        case WorldSizeType::DEMO:
            return {50 * Constants::FEET_PER_TILE, 50 * Constants::FEET_PER_TILE};
        case WorldSizeType::TINY:
            return {120 * Constants::FEET_PER_TILE, 120 * Constants::FEET_PER_TILE};
        case WorldSizeType::MEDIUM:
            return {180 * Constants::FEET_PER_TILE, 180 * Constants::FEET_PER_TILE};
        case WorldSizeType::GIANT:
            return {240 * Constants::FEET_PER_TILE, 240 * Constants::FEET_PER_TILE};
        default:
            // TODO: Handle error case
            return {-1, -1};
        }
    }

    Size getWorldSizeInTiles() const
    {
        auto worldSize = getWorldSize();
        return {worldSize.width / Constants::FEET_PER_TILE,
                worldSize.height / Constants::FEET_PER_TILE};
    }

    bool isFullscreen() const
    {
        return m_isFullscreen;
    }

    bool isVSync() const
    {
        return m_isVSync;
    }

    float getVolume() const
    {
        return m_soundVolume;
    }

    float getMusicVolume() const
    {
        return m_musicVolume;
    }

    std::string getTitle() const
    {
        return m_title;
    }

    int getViewportMovingSpeed() const
    {
        return m_viewportMovingSpeed;
    }

    int getTicksPerSecond() const
    {
        return m_ticksPerSecond;
    }

    int getTargetFPS() const
    {
        return m_targetFPS;
    }

    RevealStatus getFOWRevealStatus() const
    {
        return m_fowTRevealStatus;
    }

    void setFOWRevealStatus(RevealStatus mode)
    {
        m_fowTRevealStatus = mode;
    }

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
};
} // namespace ion

#endif