#ifndef RESOURCELOADER_H
#define RESOURCELOADER_H

#include "GameSettings.h"
#include "GameState.h"
#include "GraphicsRegistry.h"
#include "Renderer.h"
#include "SubSystem.h"

namespace game
{
class ResourceLoader : public aion::SubSystem
{
  public:
    ResourceLoader(std::stop_token* stopToken,
                   const aion::GameSettings& settings,
                   aion::GraphicsRegistry& graphicsRegistry,
                   aion::Renderer& renderer);
    ~ResourceLoader() = default;

    void loadTextures();
    void loadEntities();

  private:
    // SubSystem methods
    void init() override;

    void shutdown() override
    {
        // Cleanup code for resource loading
    }

    const aion::GameSettings& _settings;
    aion::GraphicsRegistry& graphicsRegistry;
    aion::Renderer& renderer;
};
} // namespace game

#endif