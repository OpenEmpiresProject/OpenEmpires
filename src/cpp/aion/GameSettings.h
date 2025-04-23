#ifndef GAMESETTINGS_H
#define GAMESETTINGS_H

#include "Constants.h"
#include "Types.h"
#include "WidthHeight.h"

namespace aion
{
class GameSettings
{
  public:
    GameSettings() = default;
    ~GameSettings() = default;

    void setResolution(int width, int height)
    {
        resolution.width = width;
        resolution.height = height;
    }
    void setFullscreen(bool fullscreen)
    {
        _isFullscreen = fullscreen;
    }
    void setVSync(bool vsync)
    {
        _isVSync = vsync;
    }
    void setVolume(float volume)
    {
        soundVolume = volume;
    }
    void setMusicVolume(float volume)
    {
        musicVolume = volume;
    }
    void setWindowDimensions(int width, int height)
    {
        windowDimensions.width = width;
        windowDimensions.height = height;
    }
    void setWorldSizeType(utils::WorldSizeType size)
    {
        worldSizeType = size;
    }
    void setTitle(const std::string& title)
    {
        this->title = title;
    }

    void setViewportMovingSpeed(int speed)
    {
        viewportMovingSpeed = speed;
    }

    void setTicksPerSecond(int tps)
    {
        ticksPerSecond = tps;
    }

    const utils::WidthHeight& getResolution() const
    {
        return resolution;
    }
    const utils::WidthHeight& getWindowDimensions() const
    {
        return windowDimensions;
    }
    utils::WorldSizeType getWorldSizeType() const
    {
        return worldSizeType;
    }
    const utils::WidthHeight& getWorldSize() const
    {
        switch (worldSizeType)
        {
        case utils::WorldSizeType::DEMO:
            return {50 * utils::Constants::FEET_PER_TILE, 50 * utils::Constants::FEET_PER_TILE};
        case utils::WorldSizeType::TINY:
            return {120 * utils::Constants::FEET_PER_TILE, 120 * utils::Constants::FEET_PER_TILE};
        case utils::WorldSizeType::MEDIUM:
            return {180 * utils::Constants::FEET_PER_TILE, 180 * utils::Constants::FEET_PER_TILE};
        case utils::WorldSizeType::GIANT:
            return {240 * utils::Constants::FEET_PER_TILE, 240 * utils::Constants::FEET_PER_TILE};
        default:
            // TODO: Handle error case
            return {-1, -1};
        }
    }
    utils::WidthHeight getWorldSizeInTiles() const
    {
        auto worldSize = getWorldSize();
        return {worldSize.width / utils::Constants::FEET_PER_TILE,
                worldSize.height / utils::Constants::FEET_PER_TILE};
    }
    bool isFullscreen() const
    {
        return _isFullscreen;
    }
    bool isVSync() const
    {
        return _isVSync;
    }
    float getVolume() const
    {
        return soundVolume;
    }
    float getMusicVolume() const
    {
        return musicVolume;
    }
    std::string getTitle() const
    {
        return title;
    }

    int getViewportMovingSpeed() const
    {
        return viewportMovingSpeed;
    }

    int getTicksPerSecond() const
    {
        return ticksPerSecond;
    }

  private:
    utils::WidthHeight resolution{800, 600};
    utils::WidthHeight windowDimensions{800, 600};
    utils::WorldSizeType worldSizeType = utils::WorldSizeType::DEMO;
    bool _isFullscreen = false;
    bool _isVSync = true;
    float soundVolume = 1.0f;
    float musicVolume = 1.0f;
    std::string title = "openEmipires";
    int viewportMovingSpeed = 100;
    int ticksPerSecond = 60;
};
} // namespace aion

#endif