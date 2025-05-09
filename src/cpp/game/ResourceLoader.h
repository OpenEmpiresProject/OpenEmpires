#ifndef RESOURCELOADER_H
#define RESOURCELOADER_H

#include "GameSettings.h"
#include "GameState.h"
#include "GraphicsRegistry.h"
#include "Renderer.h"
#include "SubSystem.h"

#include <memory>
namespace game
{
class ResourceLoader : public aion::SubSystem
{
  public:
    ResourceLoader(std::stop_token* stopToken,
                   const aion::GameSettings& settings,
                   aion::GraphicsRegistry& graphicsRegistry,
                   std::shared_ptr<aion::Renderer> renderer);
    ~ResourceLoader() = default;

    void loadEntities();

  private:
    // SubSystem methods
    void init() override;

    void shutdown() override
    {
        // Cleanup code for resource loading
    }

    const aion::GameSettings& m_settings;
    aion::GraphicsRegistry& m_graphicsRegistry;
    std::shared_ptr<aion::Renderer> m_renderer;
};
} // namespace game

#endif