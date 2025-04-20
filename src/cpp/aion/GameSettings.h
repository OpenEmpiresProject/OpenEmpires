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
    void setFullscreen(bool fullscreen) { _isFullscreen = fullscreen; }
    void setVSync(bool vsync) { _isVSync = vsync; }
    void setVolume(float volume) { soundVolume = volume; }
    void setMusicVolume(float volume) { musicVolume = volume; }
    void setWindowDimensions(int width, int height)
    {
        windowDimensions.width = width;
        windowDimensions.height = height;
    }
    void setWorldSizeType(utils::WorldSizeType size) { worldSizeType = size; }
    void setTitle(std::string title) { this->title = title; }

    utils::WidthHeight getResolution() const { return resolution; }
    utils::WidthHeight getWindowDimensions() const { return windowDimensions; }
    utils::WorldSizeType getWorldSizeType() const { return worldSizeType; }
    utils::WidthHeight getWorldSize() const
    {
        switch (worldSizeType)
        {
        case utils::WorldSizeType::TINY:
            return {120 * utils::Constants::TILE_SIZE, 120 * utils::Constants::TILE_SIZE};
        case utils::WorldSizeType::MEDIUM:
            return {180 * utils::Constants::TILE_SIZE, 180 * utils::Constants::TILE_SIZE};
        case utils::WorldSizeType::GIANT:
            return {240 * utils::Constants::TILE_SIZE, 240 * utils::Constants::TILE_SIZE};
        default:
            // TODO: Handle error case
            return {-1, -1};
        }
    }
    utils::WidthHeight getWorldSizeInTiles() const
    {
        auto worldSize = getWorldSize();
        return {worldSize.width / utils::Constants::TILE_SIZE,
                worldSize.height / utils::Constants::TILE_SIZE};
    }
    bool isFullscreen() const { return _isFullscreen; }
    bool isVSync() const { return _isVSync; }
    float getVolume() const { return soundVolume; }
    float getMusicVolume() const { return musicVolume; }
    std::string getTitle() const { return title; }

  private:
    utils::WidthHeight resolution{800, 600};
    utils::WidthHeight windowDimensions{800, 600};
    utils::WorldSizeType worldSizeType = utils::WorldSizeType::TINY;
    bool _isFullscreen = false;
    bool _isVSync = true;
    float soundVolume = 1.0f;
    float musicVolume = 1.0f;
    std::string title = "openEmipires";
};
} // namespace aion

#endif