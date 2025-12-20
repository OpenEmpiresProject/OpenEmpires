#include "Settings.h"

void core::Settings::setResolution(int width, int height)
{
    m_resolution.width = width;
    m_resolution.height = height;
}

void core::Settings::setFullscreen(bool fullscreen)
{
    m_isFullscreen = fullscreen;
}

void core::Settings::setVSync(bool vsync)
{
    m_isVSync = vsync;
}

void core::Settings::setVolume(float volume)
{
    m_soundVolume = volume;
}

void core::Settings::setMusicVolume(float volume)
{
    m_musicVolume = volume;
}

void core::Settings::setWindowDimensions(int width, int height)
{
    m_windowDimensions.width = width;
    m_windowDimensions.height = height;
}

void core::Settings::setWorldSizeType(WorldSizeType size)
{
    m_worldSizeType = size;
}

void core::Settings::setTitle(const std::string& m_title)
{
    this->m_title = m_title;
}

void core::Settings::setViewportMovingSpeed(int speed)
{
    m_viewportMovingSpeed = speed;
}

void core::Settings::setTicksPerSecond(int tps)
{
    m_ticksPerSecond = tps;
}

void core::Settings::setTargetFPS(int fps)
{
    m_targetFPS = fps;
}

const core::Size& core::Settings::getResolution() const
{
    return m_resolution;
}

const core::Size& core::Settings::getWindowDimensions() const
{
    return m_windowDimensions;
}

core::WorldSizeType core::Settings::getWorldSizeType() const
{
    return m_worldSizeType;
}

core::Size core::Settings::getWorldSize() const
{
    switch (m_worldSizeType)
    {
    case WorldSizeType::TEST:
        return {10 * Constants::FEET_PER_TILE, 10 * Constants::FEET_PER_TILE};
    case WorldSizeType::DEMO:
        return {50 * Constants::FEET_PER_TILE, 50 * Constants::FEET_PER_TILE};
    case WorldSizeType::TINY:
        return {120 * Constants::FEET_PER_TILE, 120 * Constants::FEET_PER_TILE};
    case WorldSizeType::MEDIUM:
        return {180 * Constants::FEET_PER_TILE, 180 * Constants::FEET_PER_TILE};
    case WorldSizeType::GIANT:
        return {240 * Constants::FEET_PER_TILE, 240 * Constants::FEET_PER_TILE};
    default:
        throw std::invalid_argument("Invalid world size type");
    }
}

core::Size core::Settings::getWorldSizeInTiles() const
{
    auto worldSize = getWorldSize();
    return {worldSize.width / Constants::FEET_PER_TILE,
            worldSize.height / Constants::FEET_PER_TILE};
}

bool core::Settings::isFullscreen() const
{
    return m_isFullscreen;
}

bool core::Settings::isVSync() const
{
    return m_isVSync;
}

float core::Settings::getVolume() const
{
    return m_soundVolume;
}

float core::Settings::getMusicVolume() const
{
    return m_musicVolume;
}

std::string core::Settings::getTitle() const
{
    return m_title;
}

int core::Settings::getViewportMovingSpeed() const
{
    return m_viewportMovingSpeed;
}

int core::Settings::getTicksPerSecond() const
{
    return m_ticksPerSecond;
}

int core::Settings::getTargetFPS() const
{
    return m_targetFPS;
}

core::RevealStatus core::Settings::getFOWRevealStatus() const
{
    return m_fowTRevealStatus;
}

void core::Settings::setFOWRevealStatus(RevealStatus mode)
{
    m_fowTRevealStatus = mode;
}

uint32_t core::Settings::getMaxPopulation() const
{
    return m_maxPopulation;
}

float core::Settings::getGameSpeed() const
{
    return m_gameSpeed;
}

void core::Settings::setGameSpeed(float val)
{
    m_gameSpeed = val;
}
